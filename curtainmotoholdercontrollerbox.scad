d1=20.5; //diameter of ballpulley
ds=3;
sw =31;
w1=42;
h1=25;
mfn=64;
slide=0;
spw=8;
fn1=16; //16
ii=0;
bottomthickness = 4;
sidethickness = 6;
sideheight = 20;
module fd(he) {
  helemets =he;
  esp =[23,24,helemets];
  miniproanddrv = [26,33,helemets];
  drv=[16,21,helemets];
  bec=[23,33,helemets];
  inx=drv[0]+2;
  iny=0;
  hull() {
    translate([-5,70,0]) {
        esp = esp + [2,2,2];
        cube(esp);
    }
    translate([-5,0,0]) {
        bec = bec + [2,2,2];
        cube(bec);
    }
  }
  hull() {
    translate([0,0,0]) {
        bec = bec + [2,2,2];
        cube(bec);
    }
    translate([40,0,0]) {
      drv = drv + [2,2,2];
      miniproanddrv = miniproanddrv + [2,2,2];
      cube(miniproanddrv);
    }
  }
}

mhheight=25;
module motorandelectronicmount(motorheight=25, holes=1) {
  mhheight=motorheight;
  difference() {
    union() {
      minkowski() {
        fd(sideheight);
        cylinder(r=sidethickness,h=0.1,$fn=32);
      }
      translate([12,35,0]) {
        translate([0,2,0]) {
          minkowski() {
            cube([(spw*2)+w1,w1-4,5]);
            cylinder(r=2,h=0.01,$fn=fn1);
          }
        }
        translate([0,12,0]) {
          minkowski() {
            cube([8,25,5]);
            cylinder(r=2,h=0.01,$fn=fn1);
          }
        }
        translate([spw,0,0]) {
          cube([w1,w1+slide,mhheight]);

      }

    }
  }
  if (holes == 1) {
    pp = [9.5,11.5];
    translate([2,-sidethickness,bottomthickness]) {
      cube([pp[0], pp[0], pp[1]]);
    }
    lh = [1.5,11];
    translate([65,34,sideheight-lh[1]]) {
      cube([lh[0], lh[1], 14.2]);
    }
    translate([0,0,bottomthickness]) {
      fd(18);
    }
    translate([12,35,0]) {
      for (x = [0+(spw/2),w1+spw+(spw/2)]) {
        //ii=2;
        ii =(x/2) +3;
        translate([x,(12+12.5) ,0]) {
          translate([0,0,2.1]){
            cylinder(r1=2,r2=4.2,h=3,$fn=fn1);
          }
          cylinder(r=2,h=12,$fn=fn1);
        }
        echo(ii);
         //ii =(x/2) +3;
       }
       translate([spw,0,0]) {
         translate([w1/2,w1/2,0]) {
           hull() {
             cylinder(d=d1,h=h1, $fn=fn1*4);
             translate([0,30,0]) {
               cylinder(d=d1,h=h1,$fn=fn1);
             }
           }
           translate([0,0,h1-3])
           cylinder(d=30,h=3, $fn=fn1*4);
         }
         for (x = [(w1-sw)/2, (w1-sw)/2 + sw]) {

           for (y = [(w1-sw)/2, (w1-sw)/2 + sw]) {
             translate([x, y, 0]) {
               translate([0,slide/2,0]) {
                 cylinder(d=7,h=6,$fn=fn1);
               }
               hull() {
                 cylinder(d=ds,h=h1, $fn=fn1*4);
               }
               translate([0,0,5]) {
                 hull() {
                   cylinder(d=7,h=14, $fn=fn1*4);
                 }
               }
             }
           }
         }
       }
    }
}
  }
}
module mh() {
  difference() {
    #union() {
    }


 }
}
motorandelectronicmount(motorheight=25,holes=1);
translate([-100,0,0]) {
  difference() {
    translate([0,0,sideheight]) {
      minkowski() {
        fd(2.5);
        difference() {
          sphere(r=sidethickness,h=0.1,$fn=32);
          translate([0,0,-sidethickness]) {
            cylinder(r=sidethickness+1, h=sidethickness);
          }
        }
      }
    }
    motorandelectronicmount(motorheight=35,holes=1);
  }
}
