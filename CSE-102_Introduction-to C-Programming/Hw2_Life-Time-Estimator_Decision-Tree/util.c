#include <stdio.h>
#include "util.h"
#include <math.h>


/* Example decision tree - see the HW2 description */
int dt0(int t, double p, double h, char s, int w) {
    int r = 0;
    if (t>35 && w!=3) r = 1;
    else if (t<=35 && s==0) r = 1; /* we have to take char s beacuse funtion protoype is this :( */
    return r;
}


char dt1a(double PL, double PW, double SL, double SW) {
    if (PL<2.45) return 'S';
    else if (PW<1.75 && PL<4.95 && PW<1.65) return 'E';
    else return 'I';
}

char dt1b(double PL, double PW, double SL, double SW) {
    if (PL<2.55) return 'S';
    else if (PW<1.69 && PL<4.85) return 'E';
    else return 'I';

}


double dt2a(double x1, double x2, double x3, int x4, int x5) {
    if (x1<31.5 ) {
        if (x2>-2.5) return (5.0);
        else if ((x2-0.1)<= x1 && x1<=(x2+0.1)) return (2.1);
        return (-1.1);
    }
    else if (-1 <= x3 && x3 <= 2) return (1.4);
    else if (x4 && x5) return (-2.23);
    return (11.0);
}

double dt2b(double x1, double x2, double x3, int x4, int x5) {
    if (12 < x1 && x1<22 ) {
        if (x3 > 5/3) return (-2.0);
        else if ((x1-0.1)<= x3 && x3<=(x1+0.1)) return (1.01);
        return (-8.0);
    }
    else if (x4 && x5) return (-1.0);
    else if ((-1)<= x2 && x2 <= 2) return ((-1.0)/(7.0));
    return (sqrt(2.0)/3.0);
}


double dt3a(double alc, double exercise, int smoker, char think, char gender) {
    /*This function calculate your lifetime quickly*/
    if (gender == 69 || gender == 101) {
        if (alc > 52.0) {
            if (smoker) {
                return (30.0);
                }
            else {
                return (45.0);
            }
        }// alc 52 if
        else {
            if (exercise >= 5.0) {
                if (smoker) {
                    return (65.0);
                }
                else {
                    return (85.0);
                }
            }// exercise
            else {
                return (75.0);
            }//exercise else
        }//smoker else end
        
    } // Gender Man


    else  if (gender == 75 || gender == 107){
        if (think == 80 || think == 112) {
            return (25.0);
        }//pesimisstic


        else if (think == 82 || think == 114) {
            if (exercise<6.5) {
 
                    if (alc > 20.0) {
                        return (85.0);
                    }

                    else {
                        return (45.0);
                    }
            }// exercise<6.5
        }//realist


        else if (think == 86 || think == 118) {
            if (smoker) {
                    return (60.0);
                }
                else {//smoker else
                    if (alc > 20.0) {
                        return (65.0);
                    }

                    else {
                        return (85.0);
                    }

                }//smoker else end
        }//pragmatist


        else { // optimistic and normal 
            if (exercise<6.5) {
                if (smoker) {
                    return (50.0);
                }
                else {//smoker else
                    if (alc > 20.0) {
                        return (65.0);
                    }

                    else {
                        return (75.0);
                    }

                }//smoker else end
            }// exercise<6.5
            else {// exercise<6.5 else
                if (alc<40) {
                    return (105.0);
                }
                else {
                    return (75.0);             
                }
            }// end exercise<6.5 else
        } //  optimistic and normal
    }//gender women 


    else  if (gender == 70 || gender == 102){
        if (think == 80 || think == 112) {
        return (55.0);
        }
        else if (think == 78 || think == 110) {
            if (smoker) {
                if (alc<12.0) {
                    if (exercise > 6.0) return (76.0);
                    else return (62.0);
                }
                else return (53.0);
            }//smoker if
            else { //smoker else start
                if (alc>24.0) return (56.0);
                else {
                    if (exercise > 4.5) return (85.0);
                    else return (71.0);
                }
            }//smoker else end
        }// think optimist


        else if (think == 86 || think == 118) {
            if (alc > 52.0) {
                if (smoker) {
                    return (30.0);
                    }
                else {
                    return (45.0);
                }
            }// alc 52 if
            else {
                return(90.0);
            }
        }//pragmatistic


        else {//other thinkings
            if (alc<8.0) {
                if (smoker) {
                    return (45.0);
                }
                else {//smoker else
                    if (exercise > 3.0) {
                        return (60.0);
                    }

                    else {
                        return (55.0);
                    }

                }//smoker else end
            }// alchol < 8
            else {// alchol < 8 else
                if (17<alc && alc<376) {
                    return (40.0);
                }
                else {
                    if (smoker) {
                        return (40.0);
                    }
                    else {
                        return (50.0);
                    }
                }
            }// alchol < 8 else
        }//other thinkings

    }//gender ferm 



    else  if (gender == 72 || gender == 104){
        if (think == 80 || think == 112) {
            return (35.0);
        }
        else if (exercise<5.5) {
            if (smoker) {
                return (45.0);
            }
            else {//smoker else
                if (alc > 18.0) {
                    return (85.0);
                }

                else {
                    return (65.0);
                }

            }//smoker else end
        }// exercise<5.5
        else {// exercise<5.5 else
            if (alc<40) {
                return (105.0);
            }
            else {
                return (75.0);             
            }
        }// end exercise<5.5 else
    }//gender herm 


    else  if (gender == 77 || gender == 109){
        return (45.0); 
    }//gender merm 
}

double dt3b(double alc, double exercise, int smoker, char think, char gender) {
    /*This function calculate your lifetime more complex*/
    if (gender == 69 || gender == 101) {
        if (think == 80 || think == 112) {
        return (45.0);
        }
        else if (think == 78 || think == 110) {
            if (smoker) {
                if (alc<12.0) {
                    if (exercise > 6.0) return (76.0);
                    else return (62.0);
                }
                else return (53.0);
            }//smoker if
            else { //smoker else start
                if (alc>24.0) return (56.0);
                else {
                    if (exercise > 4.5) return (85.0);
                    else return (71.0);
                }
            }//smoker else end
        }// think optimist
        else return (80.0);
    }// Gender Man

    else if (gender == 75 || gender == 107) {
        if (think == 80 || think == 112) {
            return (27.0);
        }
        else if (alc<10.0) {
            if (smoker) {
                return (50.0);
            }
            else {//smoker else
                if (exercise > 4.0) {
                    return (105.0);
                }

                else {
                    return (85.0);
                }

            }//smoker else end
        }//8
        else {//8 else
            if (20<alc && alc<400) {
                return (42.0);
            }
            else {
                if (smoker) {
                    return (45.0);
                }
                else {
                    return (75.0);
                }
            }
        }// alchol < 10 else
    }// Gender woMan


    else  if (gender == 70 || gender == 102){
        if (think == 80 || think == 112) {
        return (55.0);
        }
        else if (think == 78 || think == 110) {
            if (smoker) {
                if (alc<12.0) {
                    if (exercise > 6.0) return (76.0);
                    else return (62.0);
                }
                else return (53.0);
            }//smoker if
            else { //smoker else start
                if (alc>24.0) return (56.0);
                else {
                    if (exercise > 4.5) return (85.0);
                    else return (71.0);
                }
            }//smoker else end
        }// think optimist


        else if (think == 86 || think == 118) {
            if (alc > 52.0) {
                if (smoker) {
                    return (30.0);
                    }
                else {
                    return (45.0);
                }
            }// alc 52 if
            else {
                return(90.0);
            }
        }//pragmatistic


        else {//other thinkings
            if (alc<8.0) {
                if (smoker) {
                    if (alc<12.0) {
                        if (exercise > 6.0) return (54.0);
                        else return (46.0);
                    }
                    else return (43.0);
                }
                else {//smoker else
                    if (exercise > 3.0) {
                        return (60.0);
                    }

                    else {
                        if (alc<9.0) {
                            if (exercise > 4.0) return (65.0);
                            else return (48.0);
                        }
                        else return (62.0);
                    }

                }//smoker else end
            }// alchol < 8
            else {// alchol < 8 else
                if (17<alc && alc<376) {
                    if (smoker) {
                        return (51.0);
                    }
                    else {//smoker else
                        if (alc > 30.0) {
                            return (62.0);
                        }

                        else {
                            return (72.0);
                        }

                    }//smoker else end
                }
                else {
                    if (smoker) {
                        return (36.0);
                    }
                    else {
                        return (47.0);
                    }
                }
            }// alchol < 8 else
        }//other thinkings

    }//gender ferm 


    else  if (gender == 72 || gender == 104){
        if (think == 80 || think == 112) {
            return (35.0);
        }//pessimistic

    
        else if (think == 86 || think == 118) {
            if (exercise<5.5) {
                if (smoker) {
                    return (47.0);
                }
                else {//smoker else
                    if (alc > 15.0) {
                        return (78.0);
                    }

                    else {
                        return (62.0);
                    }

                }//smoker else end
            }// exercise<5.5
            else {// exercise<5.5 else
                if (alc<40) {
                    return (92.0);
                }
                else {
                    return (73.0);             
                }
            }// end exercise<5.5 else
        }//pragmatistic


        else if (think == 78 || think == 110) {// think optimist
            if (smoker) {
                if (alc<12.0) {
                    if (exercise > 6.0) return (76.0);
                    else return (72.0);
                }
                else return (63.0);
            }//smoker if
            else { //smoker else start
                if (alc>30.0) return (62.0);
                else {
                    if (exercise > 4.5) return (87.0);
                    else return (73.0);
                }
            }//smoker else end
        }// think optimist


        else {//other thinkings
            if (smoker) {
                if (alc<12.0) {
                    if (exercise > 6.0) return (76.0);
                    else return (62.0);
                }
                else return (53.0);
            }//smoker if
            else { //smoker else start
                if (alc>24.0) return (56.0);
                else {
                    if (exercise > 4.5) return (85.0);
                    else return (71.0);
                }
            }//smoker else end
        }//other thinkings
    }//gender herm 


    else  if (gender == 77 || gender == 109){
        if (think == 80 || think == 112) {
            return (17.0);
        }
        else if (exercise<6.5) {
            if (smoker) {
                return (31.0);
            }
            else {//smoker else
                if (alc > 20.0) {
                    return (42.0);
                }

                else {
                    return (54.0);
                }

            }//smoker else end
        }// exercise<6.5
        else {// exercise<6.5 else
            if (alc<40) {
                return (67.0);
            }
            else {
                return (52.0);             
            }
        }// end exercise<6.5 else
    }//gender merm 

    else return (5.0);
}

/* Provide your implementations for all the requested functions here */
