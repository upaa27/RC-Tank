# RC-Tank

-3 phase brushless motor controller

This controller is to be fed with PWM(Pulse Width Modulation) as well as a bit used for direction selection.

# Issues
Currently, the code in timer 1's interrupt service routine takes too much time and leads to PD2 and 7 having a near zero phase difference.  This can be fixed with either adding some delays, having a smart person optimize the code, by using some integrated circuit logic elements, or by using an IC designed for 3 phase motor brushless motor control(:p).
