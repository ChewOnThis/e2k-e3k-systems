START

IF Top Limit Switch is pressed // Check bridge top and bottom limits
    STOP bridge
ELSE IF Bottom Limit Switch is pressed
    STOP bridge
ELSE
    ACTIVATE bridge
END
// E-stop, override, pressure plates, ultrasonic/IR
