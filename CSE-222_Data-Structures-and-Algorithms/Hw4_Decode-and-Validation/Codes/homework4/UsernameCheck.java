package homework4;


public class UsernameCheck {
    // This variable have Default Access Modifier (package level accesebility)
    static String username = "sibelgulmez";

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
     * Checks if a given username contains only letters and at least 1 character 
     * This function have Default Access Modifier (package level accesebility)
     * 
     * @param username The string to be checked for username validity.
     * @return {@code true} if the given string is a valid username, {@code false} otherwise.
     */
    static boolean checkIfValidUsername(String username) {
        // checks string validation and lenght
        if(username == null || username.length() == 0) {
            System.out.println("the username is invalid, it should be at least 1 character");
            return false;
        }
        // checks first char is letter or not
        if(isLetter(username.charAt(0))) {
            // if last letter valid string is valid
            if(username.length() == 1)
            return true;
            // checks other letters
            return checkIfValidUsername(username.substring(1));
        }
        System.out.println("the username is invalid, it shouldcontains only letters");
        return false;
    }

    /**
     * Contructor that takes username and set 
     * class's static variable to that
     * not usefull beacuse we don't use this varibale
     * 
     * @param username2 string to set username
     */
    public UsernameCheck(String username2) {
        username = username2;
    }

    /**
     * No paramater Constructor, doing nothing 
     * 
     */
    public UsernameCheck() { // do nothing
    }

    
}
