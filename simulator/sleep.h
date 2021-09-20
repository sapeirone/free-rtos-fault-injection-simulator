#define SLEEP_SUCCESS 0
#define SLEEP_FAILURE 1

extern int eventIsSet;

void sleepNanoseconds (unsigned long ns);
void injectorWait();
void wakeInjector();
