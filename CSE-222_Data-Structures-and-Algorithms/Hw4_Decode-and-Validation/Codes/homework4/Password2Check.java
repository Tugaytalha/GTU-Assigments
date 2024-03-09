package homework4;


public class Password2Check {
    private static int intPassword = 2020;
    
    /**
     * Determines if it is possible to obtain the password
     *  by the summation of denominations along with arbitrary coefficients
     * 
     * @param password2 the integer value to be formed from the denominations array
     * @param denominations the array of integer denominations that can be used to obtain password2
     * @param currentsum the current summation of denominations 
     * @param index the index of the current denomination 
     * 
     * @return {@code true} if the password2 can be formed exactly using elements from the denominations array, {@code false} otherwise
     */
    private static boolean  isExactDivisionRecursive(int  password2,  int  []  denominations, int currentsum, int index) {
        if(index != -1)
            currentsum += denominations[index];
        if (currentsum == password2)
            return true;
        if (currentsum > password2)
            return false;
        
        for (int i =  0; i < denominations.length; ++i) {
            if(isExactDivisionRecursive(password2, denominations, currentsum, i))
                return true;
        }
        return false;
    }

    /**
     * Determines if it is possible to obtain the password
     * by the summation of denominations along with arbitrary coefficients
     * by calling a recursive funvtion after check password2's validity
     * 
     * @param password2 the integer value to be formed from the denominations array
     * @param denominations the array of integer denominations that can be used to obtain password2

     * @return {@code true} if the password2 can be formed exactly using elements from the denominations array, {@code false} otherwise
     */
    static boolean isExactDivision(int  password2,  int  []  denominations) {
        // Check if password between 10 and 10000
        if(password2 < 10 || password2 > 10000) {
            System.out.println("the password2 is invalid, it should be between 10 and 10000.");
            return false;
        }

        // Checks if password2 obtainable by summing denomination numbers or not
        if (!isExactDivisionRecursive(password2, denominations, 0, -1)) {
            System.out.println("the password1 is invalid, it should be obtainable by summing demoniations.");
            return false;
        }

        return true;
    }

    /**
     * Contructor that takes password and set 
     * class's static variable to that
     * not usefull beacuse we don't use this varibale
     * 
     * @param password string to set stringPassword
     */
    public Password2Check(int password) {
        intPassword = password;
    }

    /**
     * No paramater Constructor, doing nothing 
     * 
     */
    public Password2Check() { // do nothing
    }

}
