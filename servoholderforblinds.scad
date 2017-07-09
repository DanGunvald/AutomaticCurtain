outer = [28,24.5,22];
softcorner=3;
servo=[23,12.4,23];

difference() {
    translate([0,softcorner/2,softcorner/2]) {
        minkowski() {
            cube(outer-[0,softcorner,softcorner]);
            rotate([0,90,0]) {
                cylinder(d=softcorner,h=0.1,$fn=16);
            }
        }
    }
    translate([0,(outer[1] - servo[1]) /2,0]) {
        #cube(servo);
    }
}