New feature: Add axis support

In this new feature we ill add:
+ 100 new axis custom commands. i.e. MultiBind Command A00-A99

These new axis commands will allow axis assignments from the xplane joystick menus
Each assignment will be a dataref, the same way the buttons look in the cfg file.
such as: `*010xA01=sim/egnines/throttle` (Not a verfied datref, just an example)
`A01` = Being an axis id 
`x` = checking for axis assignment

This would allow the flow:
- user holds`(*)` 010
- This activates any dataref assigned to *010xA01.
- for each assigned dataref, capture the current axis value from controller and assign it to the dataref. 

From the user, when button `010` is held, the throttle moves to the location of the axis `A01`
