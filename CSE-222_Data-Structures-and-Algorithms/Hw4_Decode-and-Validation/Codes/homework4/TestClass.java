package homework4;

public class TestClass {
    private static int testNumber = 0;
   
    public static boolean test(String username, String password1, int password2, int[] denominations) {
        ++testNumber;
        System.out.println("Test " + testNumber + "... inputs:");
        System.out.println("username: \"" + username + "\" - password1: \"" + password1 + "\" - password2: " + password2);

        if(UsernameCheck.checkIfValidUsername(username) &&
            Password1Check.isBalancedPassword(password1) &&
            Password1Check.containsUserNameSpirit(username, password1) &&
            Password1Check.isPalindromePossible(password1) &&
            Password2Check.isExactDivision(password2, denominations) 
        ) {
            System.out.println("The username and passwords are valid. the Door is opening. Please wait...");
            System.out.println();
            return true;
        }
        
        System.out.println();
        return false;
    }

}
