dnl Copyright (c) 2005 Umetrics AB
dnl Part of SIMCA-Q example code. 

AC_DEFUN([AC_PATH_SIMCAQ],
[
   AC_MSG_CHECKING(for SIMCA-Q)

   ac_sqp="no"

   AC_ARG_WITH(simcaq,
     [  --with-simcaq     where SIMCA-Q is located. ],
     [  ac_sqp="$withval" ])

   dnl Did the user give --with-simcaq
    
   if test "$ac_sqp" = no; then
       dnl No they didn't, so lets look for them...
    
       if test "x$SIMCAQDIR" != x; then
          ac_sqp="$SIMCAQDIR"
       else
          dnl Check if sqp can be found globally
          dnl If not, try to find sqp in various places.
          AC_CHECK_LIB(simcaq, InitSQPFloatMatrix, 
                       [SQPLIBS="-lsimcaq"
                        have_sqp="yes"],
                       [sqp_library_dirs="\
                                           /usr/local/lib \
                                           ../../unix/.libs \
                                           ../.libs \
                                           ../../libs/predictor/interface \
                                           ./../../libs/predictor/interface \
                                           ../../.libs \
                                           /usr/lib/simcaq/lib \
                                           /usr/local/lib/simcaq/lib"
                         for sqp_dir in $sqp_library_dirs; do
                             for sqp_check_lib in $sqp_dir/libsimcaq.so*; do
                                 if test -r $sqp_check_lib; then
                                     echo Found lib $sqp_dir
                                     sqp_libraries=$sqp_dir
                                     have_sqp="yes"
                                     SIMCAQLIBS="-lsimcaq"
                                     SIMCAQLDFLAGS=-L$sqp_dir
                                     break 2
                                 fi
                             done
                         done])
          AC_CHECK_HEADER(SQPCInterface.h,
                          [have_sqp="yes"],
                          [sqp_include_dirs="\
                                              /usr/local/include \
                                              ../../libs/predictor/interface \
                                              ../../../libs/predictor/interface"
                            for sqp_dir in $sqp_include_dirs; do
                               if test -r $sqp_dir/SQPCInterface.h; then
                                   echo Found include $sqp_dir
                                   have_sqp="yes"
                                   SIMCAQCPPFLAGS=-I$sqp_dir
                                   break
                               fi
                            done])
       fi
   else
           have_sqp="yes"

           SIMCAQLDFLAGS="-L$ac_sqp/lib"
           SIMCAQLIBS="-lsimcaq"
           SIMCAQCPPFLAGS="-I$ac_sqp/include"
   fi
   AC_MSG_RESULT([$ac_sqp])
   AC_SUBST(SIMCAQLIBS)
   AC_SUBST(SIMCAQLDFLAGS)
   AC_SUBST(SIMCAQCPPFLAGS)
])
