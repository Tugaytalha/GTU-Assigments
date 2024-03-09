package homework4;

import java.util.Stack;

public class Password1Check {
    private static String stringPassword = "{A Hello}";
    
    /**
     * Checks if a given character is letter or not
     * 
     * @param c The character to be checked.
     * @return {@code true} if the given char is a letter, {@code false} otherwise.
     */
    private static boolean isLetter(char c) {
        if((c < 91 && c > 64) || (c < 123 && c > 96)) return true;
        return false;
    }

   /**
     * Returns the lowercase equivalent of the given character.
     * 
     * @param c the character to be converted to lowercase
     * @return the lowercase equivalent of the given character, or the original character if it is already lowercase
     */
    private static char lowerCase(char c) {
        if (c >= 'A' && c <= 'Z') {
            return (char) (c + ('a' - 'A'));
        } else {
            return c;
        }
    }

    /**
     * Checks if the given password contains at least one letter of the given username.
     * This function have Default Access Modifier (package level accesebility)
     *
     * @param username the username to check for
     * @param password1 the password to check
     * @return {@code true} if the password contains at least one letter of the username, {@code false} otherwise
     */
    static boolean containsUserNameSpirit(String username, String password1) {
        Stack<Character> stack = new Stack<Character>();

        if(password1.length() < 8) {
            System.out.println("the password1 is invalid, it should be contains at least 8 character.");
            return false;
        }
    
        // Push each character of the password onto the stack
        for (char c : password1.toCharArray()) {
          stack.push(lowerCase(c));
        }

        for(char c : username.toCharArray()) {
            if (stack.contains(lowerCase(c)))
                return true;
        }

        System.out.println("the password1 is invalid, it should be contains at least one letter of the given username. ");
        return false;
    }
    


    /**
     * In  the  given  string 
     * sequence, the function considers two brackets to be matching if the first bracket is an open 
     * bracket, (ex: (, {, or [), and the next bracket is a closed bracket of the same type. 
     * String cannot start with a closed bracket
     * This function have Default Access Modifier (package level accesebility)
     *
     * @param password1 the string to check for balanced brackets
     * @return {@code true} if the string contains balanced brackets, {@code false} otherwise
     */
    static boolean isBalancedPassword(String password1) {
        Stack<Character> stack = new Stack<>();
        boolean isAnyBracket = false, isAnyLetter = false;
        
        for (char c : password1.toCharArray()) {
            if (c == '(' || c == '{' || c == '[') {
                stack.push(c);
                isAnyBracket = true;
            } else if (c == ')' || c == '}' || c == ']') {
                if (stack.isEmpty()) {
                    System.out.println("the password1 is invalid, it should be balanced");
                    return false;
                }
                char top = stack.pop();
                if ((c == ')' && top != '(') || (c == '}' && top != '{') || (c == ']' && top != '[')) {
                    System.out.println("the password1 is invalid, it should be balanced");
                    return false;
                }
            } else if(isLetter(c)) {
                isAnyLetter = true;
            } else {
                System.out.println("the password1 is invalid, it should contains only brackets and letters");
                return false;
            } 
            
            
        }
        if (!isAnyBracket)
            System.out.println("the password1 is invalid, it should have at least 2 brackets");
        if (!isAnyLetter)
            System.out.println("the password1 is invalid, it should have at least 1 letter");
        if(!stack.isEmpty())
            System.out.println("the password1 is invalid, it should be balanced");
            
        return (stack.isEmpty() && isAnyBracket && isAnyLetter);
    }

    /**
     * Determines whether it ispalindromeable from a given password string by removing any brackets 
     * and call recursive control function. If it is possible, returns true; otherwise, returns false.
     * This function have Default Access Modifier (package level accesebility)
     *
     * @param password1 the password string to check palindromeable
     * @return {@code true} if the password can be palindromeable, {@code false} otherwise
     */
    static boolean isPalindromePossible(String password1) {
        // For more efficiency build string with string builder 
        StringBuilder output = new StringBuilder();
        String noBPass;
        
        // remove brackets from string
        for (char c : password1.toCharArray()) {
            if (c != '(' && c != ')' && c != '[' && c != ']' && c != '{' && c != '}') {
                output.append(c);
            }
        }

        Boolean[] table = new Boolean[output.length() + 1];
        // Table creation for recursive function check 
        for(int i = 0; i < output.length(); ++i) {
            table[i] = false;
        }

        // Last element of the table is carrying info that can there be another letter without a pair, if lenght is even no letter can be without a pair
        table[output.length()] = (output.length() % 2 == 0) ? false : true;
        
        // Continue as a string
        noBPass = output.toString();

        // recursive function call
        if(!recursivePolindrome(noBPass, table, 0)) {
            System.out.println("the password1 is invalid, it should be palindromable. ");
            return false;
        }
        return true;
    }

    /**
     * Recursively checks whether it is palindromeable from a given password string
     *
     * @param password1 the password string to check
     * @param table a Boolean array to keep track of whether each letter has a matching pair
     * @param index the current index being checked in the recursion
     * @return {@code true} if a palindrome can be formed, {@code false} otherwise
     */
    private static boolean recursivePolindrome(String password1, Boolean[] table, int index) {
        // if index equal to lenght everything is fine, return true
        if(index == password1.length())
        return true;
        // Aldready checked in other call
        if(table[index] == true)
            return recursivePolindrome(password1, table, index + 1);

        char c = password1.charAt(index);
        
        // Checks Is there any other copy of same letter
        for(int i = index + 1; i < password1.length(); ++i) {
            if (c == password1.charAt(i) && !table[i]) {
                table[i] = true;
                return recursivePolindrome(password1, table, index + 1); 
            }
        }

        // If lenght of the string is even 1 letter can be single
        if(table[password1.length()]) {
            table[password1.length()] = false;
            return recursivePolindrome(password1, table, index + 1); 
        }

        // Cannot be Polindrome 
        return false;
    }

    /**
     * Contructor that takes password and set 
     * class's static variable to that
     * not usefull beacuse we don't use this varibale
     * 
     * @param password string to set stringPassword
     */
    public Password1Check(String password) {
        stringPassword = password;
    }

    /**
     * No paramater Constructor, doing nothing 
     * 
     */
    public Password1Check() { // do nothing
    }
}
