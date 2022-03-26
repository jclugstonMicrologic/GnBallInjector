
/*|***************************************************************************/
/*|PROJECT:  BIC (Ball Injection Controller)
/*|***************************************************************************/

version
mm/dd/yy

v0.00      Hardware setup
05/23/12

v0.01      Beta Release
09/16/12   Motor controlled tuned
           1msec timer 50% error, causing invalid bpm count, fixed

           
v0.02      Beta Release
11/17/12   Braking immediately causing current spikes, time delay between 
           motor off, and motor brake added

           
v0.03      Beta Release
11/22/12   If brake has been applied, do not start motor for 5msec
           PWM out turned off when motor stopped 

           
v0.04       
12/dd/12


v0.05      balls/rev selection is now available, and stored in internal flash
12/08/12   loading 200balls/indexing issue sorted out


v0.06      Direction/rate are now highlighted based on what the up/down buttons control
01/26/13   If adjusting rate, back returns you to direction adjustment (used to go to main screen)
           At the end of indexing, motor can be run in reverse (jog function)
           At the end of run, motor can be run forward (jog function)
           During ejecting, motor reverse is more readily accessible
           Lagging ball count fixed (all edges are detected correctly, it was a ball counter issue)


v0.07      Display BALL INJ on main screen only, other screens display BALL INJECTOR
02/24/13   Enable off button
           Option to return to indexing if user has switched to load
           At the end of indexing, motor can be run in reverse (jog function), issue in v0.06 fixed
           At the end of run, motor can be run forward (jog function), issue in v0.06 fixed


v0.08      Load/index menus changed to allow reverse during loading or indexing
03/16/13   Once loading has started, can not return to main screen
           Once indexing has started, can not return to load screen
           Can only get to main screen once indexing complete


v0.09      In the LOAD screen, after you have loaded some balls and pressed stop, the blank key between INDEX and START
04/13/13   still decrements the rate, fixed
	   If user increments ball count in setup screen, the B.I. now assumes indexing is complete


v0.10      Pressing up or down while motor is running full speed in setup screen causes
04/dd/13   the motor to reverse immediately.  Up/down now disabled during motor operation

v0.11      Can not adjust load rate when initially enter screen, fixed
07/14/13   After index complete fwd disabled until rev is pressed, then fwd enabled until ball count back to 0
           After run/auto complete and start fwd is pressed in manual screen ball count jumps to 200, fixed
           After run complete, setup test only go forwrd 1/8th rev, fixed
           Screens displaying prev screen info fixed

v0.12      Change logo bitmap to new GN logo
08/24/13   

v0.13a     Change to 255 max ball count (JFlash Lite which does not require a license for Bob to program)
02/07/21   used 'a' because of multiple tests for Bob via email, left it this way because no big deal



