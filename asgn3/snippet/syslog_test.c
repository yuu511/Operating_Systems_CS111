#include  <syslog.h>
#include <stdarg.h>

int main(){
  int x = 5;
  syslog(LOG_ALERT, "TEST_DDDDDD %d",x);
}
