#include "demo.h"

typedef enum { false, true } bool;
struct sym_list Head;   /* head of singly-linked list */

/*
 * Daemon provides an interactive associative memory
 * via a socket-based interface. Clients either set
 * values with an assignment statement or access
 * values with a $ preface. When the value is
 * accessed, we write it onto the client's socket.
 * We currently do this as an iterative server for
 * reasons of queuing and serialization. If the
 * server is made concurrent, the database will have
 * to have serialized access and copy control - this
 * is not necessary yet.
 *
 * Program notes:
 * Parsing is done
 * with find_dollar() and find_equals().
 * Storage management is carried out by the insert()
 * and lookup() routines.
 * save() and restore() routines added to
 * use disk storage to maintain memory across
 * invocations.
 * Iterative server code is copied from Stevens, "UNIX Network
 * Programming: Networking APIs: Sockets and XTI," p. 101
 *
 */

int
main( argc, argv, env )
    int argc;
    char *argv[], *env[];
{
  int server_fd, create_service(), connection_fd;
  void service(), save();
  // restore();
  socklen_t len;
  struct sockaddr_in cliaddr;
  char buf[BUFSIZE];
  extern int close();

  server_fd = create_service();

  while( HELL_NOT_FROZEN )
    {
      len = sizeof( cliaddr );
      connection_fd = accept( server_fd, (SA *) &cliaddr, &len );

      if( connection_fd < 0 ){
      perror( "accept on server_fd" );
      exit( ERR_ACCEPT );
    }

    // restore( DATABASE );
    service(connection_fd);
    save( DATABASE );

    close( connection_fd );
    }
}

void
service( int fd ) {
    bool inChoosingOptions = true;
    bool inInsertUsername = false;
    bool inInsertPassword = false;
    char *ptr, *name, *pass;

    FILE *client_request, *client_reply, *fdopen();
    char buf[BUFSIZE];
    extern  void fix_tcl(), insert();

    /* interface between socket and stdio */
    client_request = fdopen( fd, "r" );
    
    if( client_request == (FILE *) NULL ) {
        perror( "fdopen of client_request" );
        exit( 1 );
    }

    client_reply = fdopen( fd, "w" );
  
    if( client_reply == (FILE *) NULL ) {
        perror( "fdopen of client_reply" );
        exit( 1 );
    }

    fputs( "Enter 1 to Insert New User\n Enter 2 to Update Password for Existing User", client_reply );
    fflush(client_reply);

    while( fgets( buf, BUFSIZE, client_request ) != NULL ){
        fix_tcl( buf ); /* hack to interface with tcl scripting language */
        fprintf(stderr,"%s\n",buf);
        // printf

        if(inChoosingOptions) {
            if ((ptr = find_1( buf )) != (char *) NULL) {
                fputs( "Enter Username:\n", client_reply );
                fflush( client_reply );
                inChoosingOptions = false;
                inInsertUsername = true;
            }
        }

        else if(inInsertUsername){
            name = strsave( buf );
            fputs( "Enter Password:\n", client_reply );
            fflush( client_reply );
            inInsertUsername = false;
            inInsertPassword = true;
        }

        else if (inInsertPassword) {
            pass = strsave( buf );
            fputs( "Username and Password Saved:\n", client_reply );
            fflush( client_reply );
            insert( name, pass );
            inInsertUsername = false;
            inInsertPassword = false;
            inChoosingOptions = true;
            fputs( "Enter 1 to Insert New User\n Enter 2 to Update Password for Existing User", client_reply );
            fflush(client_reply);
        }
    

          /* Create new username,password */
        // if( (ptr = find_equals( buf )) != (char *) NULL ) {
        //     fprintf(stderr,"in equals");
        //     #ifdef EBUG
        //         fprintf( stderr, "ASSIGN: %s\n", buf );
        //         dump( ptr );
        //     #endif
        //     *ptr = EOS;
        //     name = strsave( buf ); 
        //     value = strsave( ++ptr );
        //     fprintf(stderr,"%s\n",name);
        //     fprintf(stderr,"%s\n",value);
        //     insert( name, value );
        //     fputs( "\n", client_reply );
        //     fflush( client_reply );
        //     #ifdef EBUG
        //         fprintf( stderr, "REPLY: <>\n" );
        //     #endif
        // }

        /* Find username,password */ 
        // else if ((ptr = find_dollar( buf )) != (char *) NULL) {
        //     char *reply, *find_newline; 
        //     /* removes trailing newline if found */ 
        //     if( (reply = lookup( ++ptr )) != NULL ) {
        //         fputs( reply, client_reply );
        //         fflush( client_reply );
        //     }
        //     else {
        //         fputs( "No such user\n", client_reply );
        //         fflush( client_reply );
        //     }
        // }

        else {
            #ifdef EBUG
                fprintf( stderr, "GARBAGE\n" );
            #endif
        }
    }
    return;
}

int create_service()
{
  int listenfd;
  struct sockaddr_in servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0 );
  if( listenfd < 0 )
    {
      perror( "creating socket for listenfd" );
      exit( ERR_SOCKET );
    }
  
  bzero( &servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons( CIS551_PORT );

  if( bind( listenfd, (SA *) &servaddr, sizeof(servaddr) ) < 0 )
  {
    perror( "bind on listenfd");
    exit( ERR_BIND );
  }

  if( listen( listenfd, LISTENQ ) < 0 )
    {
      perror( "listen on listenfd" );
      exit( ERR_LISTEN );
    }

  return( listenfd );
}

void
fix_tcl( char *buf )
{
  char *ptr;

#define CARRIAGE_RETURN '\r'
  if( (ptr = strrchr( buf, CARRIAGE_RETURN )) != NULL )
    *ptr = EOS;
  return;

}
 
void
dump( char *buf )
{
  fprintf( stderr, "strlen(buf)=%d, buf=<%s>\n", (int) strlen(buf), buf );
  {
    int i;

    for( i=0; buf[i] != EOS; i++ )
      fprintf( stderr, "%d:%c:%x\n", i, buf[i], buf[i] );
  }
}
