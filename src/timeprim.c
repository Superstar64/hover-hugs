/*
 * Primitives needed to implement the Haskell 98 Time & CPUTime modules.
 *
 * This file has to be included by builtin.c, and won't compile on its own.
 */
 

/* --------------------------------------------------------------------------
 * Time/CPUTime control:
 * ------------------------------------------------------------------------*/

static Void timeControl Args((Int));

static Void timeControl(what)
Int what; {
}

/* --------------------------------------------------------------------------
 * Time/CPUTime primitive table:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primClockTicks);
PROTO_PRIM(primGetCPUUsage);

PROTO_PRIM(primGetClockTime);
PROTO_PRIM(primGetCalTime);
PROTO_PRIM(primMkTime);

static struct primitive timePrimTable[] = {
  /* CPUTime primitives */
  {"clockTicks",           0, primClockTicks},
  {"getCPUUsage",          0+IOArity, primGetCPUUsage},
  /* Time primitives */
  {"getClockTimePrim",     0+IOArity, primGetClockTime},        
  {"toCalTimePrim",        2+IOArity, primGetCalTime},        
  {"toClockTimePrim",      7+IOArity, primMkTime},
  {0,			0, 0}
};

static struct primInfo timePrims = { timeControl, timePrimTable, 0 };

/* --------------------------------------------------------------------------
 * Time primitives:
 * ------------------------------------------------------------------------*/

primFun(primGetClockTime) { /* :: IO (Int,Int) */
  struct timeval tv;
  int rc;
  
  rc = gettimeofday(&tv,NULL);
  
  if (rc == -1)
    throwErrno("Time.getClockTime", TRUE, NO_HANDLE, NULL);
  IOReturn(ap(ap(mkTuple(2), mkInt(tv.tv_sec)),mkInt(tv.tv_usec)));
}

primFun(primGetCalTime) { /* Int   -> Int -> IO (.....) */
                          /* isUTC    secs */
                          /* isUTC => convert time to UTC, o/wise local time */

  Int isUTC;
  time_t secs;
  struct tm* tm;
  char* zoneNm = NULL;
  Int utcOff;
  Cell zoneStr = NIL;

  IntArg(isUTC,2+IOArity);
  IntArg(secs,1+IOArity);
  
  if (isUTC) {
    tm=gmtime(&secs);
  } else {
    tm=localtime(&secs);
  }
  
  /* Warning - ugliness. */
  zoneNm = (char*)tm->tm_zone;

  utcOff = tm->tm_gmtoff;

  pushString(zoneNm);
  zoneStr = pop();

  IOReturn(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(mkTuple(11),mkInt(tm->tm_sec)),
					 mkInt(tm->tm_min)),
				      mkInt(tm->tm_hour)),
				   mkInt(tm->tm_mday)),
				mkInt(tm->tm_mon)),
			     mkInt(tm->tm_year)),
			  mkInt(tm->tm_wday)),
		       mkInt(tm->tm_yday)),
		    mkInt(tm->tm_isdst)),
		 zoneStr),
	      mkInt(utcOff)));

}

primFun(primMkTime) { /* Int{-year-}  -> Int{-month-} -> Int{-day-} ->
                         Int{-hour-}  -> Int{-mins-}  -> Int{-sec-} ->
			 Int{-tz offset-} -> IO Int{-secs since Epoch-} */
#if HAVE_MKTIME
  Int year, month, day;
  Int hour,min,sec;
  Int tz;

  struct tm tm;
  time_t t;
  
  IntArg(year,7+IOArity);
  IntArg(month,6+IOArity);
  IntArg(day,5+IOArity);
  IntArg(hour,4+IOArity);
  IntArg(min,3+IOArity);
  IntArg(sec,2+IOArity);
  IntArg(tz,1+IOArity);
  
  tm.tm_year = year;
  tm.tm_mon  = month;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min  = min;
  tm.tm_sec  = sec;
  /* The OpenGroup spec suggests that setting tm_isdst to a neg. value,
     makes mktime() try to figure this out on its own. */
  tm.tm_isdst = -1;
  
  t = mktime(&tm);
  
  if (t ==(time_t)-1)
    throwErrno("Time.toClockTime", TRUE, NO_HANDLE, NULL);

  /* mktime() assumes that the given time was local, but we might have
     been passed an UTC cal. time, so we now have to add the UTC
     offset, that is, the difference between toClockTime's UTC offset and
     the UTC offset returned by mktime().
  */
  tz = -tz;
  tz += tm.tm_gmtoff;
  
  IOReturn(mkInt(t+tz));
#else /* !HAVE_MKTIME */
  IOFail(mkIOError(NULL,
		   nameIllegal,
		   "Time.toClockTime",
		   "operation not supported",
		   NULL));
#endif
}

/* --------------------------------------------------------------------------
 * CPUTime primitives:
 * ------------------------------------------------------------------------*/

#ifdef CLK_TCK
CAFInt(primClockTicks, CLK_TCK)
#elif defined(CLOCKS_PER_SEC)
CAFInt(primClockTicks, CLOCKS_PER_SEC)
#else
CAFInt(primClockTicks, sysconf(_SC_CLK_TCK))
#endif

/* 
 * The code for grabbing the process times has been lifted from GHC.
 * Don't feel too bad about that, since I wrote and maintained it. 
 */
primFun(primGetCPUUsage) { /* IO (Int,Int,Int,Int) */
  int userSec, userNSec;
  int sysSec,  sysNSec;
    struct rusage t;

    getrusage(RUSAGE_SELF, &t);
    userSec  = t.ru_utime.tv_sec;
    userNSec = 1000 * t.ru_utime.tv_usec;
    sysSec   = t.ru_stime.tv_sec;
    sysNSec  = 1000 * t.ru_stime.tv_usec;

    IOReturn(ap(ap(ap(ap( mkTuple(4), mkInt(userSec)),
		      mkInt(userNSec)),
		   mkInt(sysSec)),
		mkInt(sysNSec)));
}

