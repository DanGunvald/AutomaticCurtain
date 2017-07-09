d1=4.4;
d2= 3;
t=2;
hei=20;
aw=4.3;
al=12+((d1+(t*2))/3);
at=2.1;
hole=8.8;


difference() {
    union() {
        cylinder(d=d1+(t*2), h=hei, $fn=64);
        translate([-(d1+(t*2))/2, -(d1+(t*2))/2,0]) {
            cube([d1+(t*2),al,t*2]);
        }
    }
    rotate([0,0,105]) {
        difference() {
            cylinder(d=d1, h=hei,$fn=16);
            translate([-d1/2,d1/4,0]) {
                cube([d1,d1,hei]);
            }
        }
    }
  
    translate([0,0,(at/2)+t/2]) {  
        cube([aw,100,at], center=true);
        cube([100,aw,at], center=true);
    }
    translate([0,hole,0]) {
        cylinder(d=2.5,h=10,$fn=16);
    }
    translate([0,0,1]) {
        cube([aw*2,aw*2,3],center=true);
        cylinder(d=7.6,h=at);
    }
}