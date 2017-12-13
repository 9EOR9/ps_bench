#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>

#define check_rc(rc) if ((rc)) return (rc);

static unsigned char use_text_protocol= 0;
static unsigned char prepare_once= 0;

static char *username= 0;
static char *hostname= 0;
static char *socket= 0;
static char *database= 0;
static int port= 0;
static char *password= 0;
static long long iterations= 10000;
static int create_data= 0;
MYSQL *mysql= 0;

static void usage()
{
  printf("ps_bench options\n");
  printf("-u username\n");
  printf("-p password\n");
  printf("-h hostname (default = localhost)\n");
  printf("-s socket\n");
  printf("-d database\n");
  printf("-i iterations\n");
  printf("-t use text protocol\n");
  printf("-P port (default 3306)\n");
  printf("-o prepare once, execute multiple\n");
  printf("-c create table and fill data\n");
  exit(-1);
}

static void set_options(int argc, char **argv)
{
  int option= 0;

  while ((option= getopt(argc, argv, ":u:p:h:P:s:d:i:toc?")) != -1)
  {
    switch(option) {
    case 'u':
      username= optarg;
      break;
    case 'd':
      database= optarg;
      break;
    case 'h':
      hostname= optarg;
      break;
    case 'p':
      password= optarg;
      break;
    case 'P':
      port= atoi(optarg);
      break;
    case 's':
      socket= optarg;
      break;
    case 't':
      use_text_protocol= 1;
      break;
    case 'c':
      create_data= 1;
      break;
    case 'o':
      prepare_once= 1;
      break;
    case 'i':
      iterations= atoll(optarg);
      break;
    default:
      usage();
      exit(-1);
    }
  }
}
int run_binary_test()
{
  int rc;
  long long i;
  MYSQL_STMT *stmt;
  printf("binary protocol\n");

  if (prepare_once)
  {
    stmt= mysql_stmt_init(mysql);
    rc= mysql_stmt_prepare(stmt, "select id1, id2, a1, a2, b1, b2, c1, c2 from bench1, bench2 where id1=id2 and id1=1", -1);
    check_rc(rc);
  }


  for (i=0; i < iterations; i++)
  {
    if (!prepare_once)
    {
      stmt= mysql_stmt_init(mysql);
      rc= mariadb_stmt_execute_direct(stmt, "select id1, id2, a1, a2, b1, b2, c1, c2 from bench1, bench2 where id1=id2 and id1=1", -1);
    }
    else
      rc= mysql_stmt_execute(stmt);
    check_rc(rc);

    rc= mysql_stmt_store_result(stmt);
    check_rc(rc);

    rc= mysql_stmt_free_result(stmt);
    check_rc(rc);
    if (!prepare_once)
      mysql_stmt_close(stmt);
  }
  if (prepare_once)
    mysql_stmt_close(stmt);
  return 0;
}

int run_text_test()
{
  int rc;
  long long i;

  printf("text protocol\n");

  for (i=0; i < iterations; i++)
  {
    MYSQL_RES *res;
    rc= mysql_query(mysql, "select id1, id2, a1, a2, b1, b2, c1, c2 from bench1, bench2 where id1=id2 and id1=1");
    check_rc(rc);

    res= mysql_store_result(mysql);
    mysql_free_result(res);
  }
  return 0;
}

int fill_data()
{
  int rc, i;

  rc= mysql_query(mysql, "DROP TABLE IF exists bench1, bench2");
  check_rc(rc);

  rc= mysql_query(mysql, "CREATE TABLE `bench1` ( `id1` int(11) NOT NULL AUTO_INCREMENT, `a1` int(11) DEFAULT NULL, `b1` varchar(36) DEFAULT NULL, `c1` varchar(36) DEFAULT NULL, PRIMARY KEY (`id1`)) ENGINE=MEMORY");
  check_rc(rc);
  rc= mysql_query(mysql, "CREATE TABLE `bench2` ( `id2` int(11) NOT NULL AUTO_INCREMENT, `a2` int(11) DEFAULT NULL, `b2` varchar(36) DEFAULT NULL, `c2` varchar(36) DEFAULT NULL, PRIMARY KEY (`id2`)) ENGINE=MEMORY");
  check_rc(rc);

  for (i=0; i < 10; i++)
  {
    rc= mysql_query(mysql, "insert into bench2 values (NULL, rand()*65536, uuid(), uuid())");
    check_rc(rc);
    rc= mysql_query(mysql, "insert into bench1 values (NULL, rand()*65536, uuid(), uuid())");
    check_rc(rc);
  }
  return 0;
}

int main(int argc, char **argv)
{
  int rc;
  set_options(argc, argv);

  /* establish connection */
  mysql= mysql_init(NULL);
  if (!(mysql= mysql_real_connect(mysql,
                                  hostname ? hostname : "localhost",
                                  username ? username : "root",
                                  password,
                                  database ? database : "test",
                                  port,
                                  socket,
                                  0)))
  {
    printf("Error: %s\n", mysql_error(mysql));
    mysql_close(mysql);
    return -1;
  }
  if (create_data)
  {
    if ((rc= fill_data()))
      printf("Error: %s\n", mysql_error(mysql));
    mysql_close(mysql);
    return rc;
  }
  rc= use_text_protocol ? run_text_test() : run_binary_test();
  if (rc)
  {
    printf("Error: %s\n", mysql_error(mysql));
    mysql_close(mysql);
    return -1;
  }

  return 0;
}
