package homework1;


/**
 * This class represents a user social media account account and it's utility
 * functions.
 */
public class Account {

    private Integer accountId;
    private String username;
    private String birthdate;
    private String location;
    private Post[] posts = new Post[50];
    private int nposts;
    private Message[] inbox = new Message[50];
    private int ninbox;
    private Message[] outbox = new Message[50];
    private int noutbox;
    private Integer[] following = new Integer[50];
    private int nfollowing;
    private Integer[] followers = new Integer[50];
    private int nfollowers;
    private Integer[] blocked = new Integer[50];
    private int nblocked;
    private Account viewing = null;
    private Boolean loggedin;
    static private int usercount = 0;

    /**
     * Creates a new account with the given username, birthdate and location.
     * 
     * @param uname  A unique username.
     * @param bdate The birthdate of the user as a string.
     * @param loc  The location of the user as a stirng.
     * @param accs account array that exists
     */
    public Account(String uname, String bdate, String loc, Account[] accs) {
        username = "take_unique_username";
        changeUName(uname, accs);
        usercount++;
        this.accountId = Integer.valueOf(usercount);
        this.birthdate = bdate;
        this.location = loc;
        this.loggedin = false;
        this.nposts = 0;
        this.noutbox = 0;
        this.ninbox = 0;
        this.nfollowing = 0;
        this.nfollowers = 0;
        this.nblocked = 0;
        accs[usercount - 1] = this;
    }

    /**
     * Returns an immutable copy of the followers array.
     * 
     * @return An array of follower account IDs.
     */
    public Integer[] getFollowers() {
        Integer[] copyFollowers = new Integer[nfollowers];
        System.arraycopy(followers, 0, copyFollowers, 0, nfollowers);
        return copyFollowers;
    }

    /**
     * returns the blocked array copy
     *
     * @return The blocked user at the specified index.
     */
    public Integer[] getBlocked() {
        Integer[] copyblocked = new Integer[nblocked];
        System.arraycopy(blocked, 0, copyblocked, 0, nblocked);
        return copyblocked;
    }

    /**
     * Getter for number of blocked users.
     *
     * @return The number of blocked users.
     */
    public int getNBlocked() {
        return nblocked;
    }


    /**
     * Returns an immutable copy of the following array.
     * 
     * @return An array of following account IDs.
     */
    public Integer[] getfollowing() {
        Integer[] copyfollowing = new Integer[nfollowing];
        System.arraycopy(following, 0, copyfollowing, 0, nfollowing);
        return copyfollowing;
    }

    /**
     * return userID as an int
     *
     * @return accountID's intValue
     */
    public int getID() {
        return accountId.intValue();
    }

    /**
     * Returns the number of users who follow this account.
     * 
     * @return The number of followers.
     */
    public int getNFollowers() {
        return nfollowers;
    }

    /**
     * Returns the number of users this account is following.
     * 
     * @return The number of accounts being followed.
     */
    public int getNFollowing() {
        return nfollowing;
    }

    /**
     * Returns the username.
     * 
     * @return The username of the account.
     */
    public String getUsername() {
        return username;
    }

    /**
     * Returns the location.
     * 
     * @return The location of the user.
     */
    public String getLocation() {
        return location;
    }

    /**
     * Returns the birthdate.
     * 
     * @return The birthdate of the user.
     */
    public String getBirthdate() {
        return birthdate;
    }

    /**
     * Returns the number of posts.
     *
     * @return The number of posts.
     */
    public int getNPosts() {
        return nposts;
    }

    /**
     * Return the post that have given index.
     *
     * @param i index for posts array.
     * @return Post at given index.
     */
    public Post getPost(int i) {
        return posts[i];
    }
    
    /**
     * Print the number of msg in outbox.
     *
     */
    public void checkNoutbox() {
        System.out.println("You have " + noutbox + " in outbox");;
    }

    /**
     * Print the number of msg in inbox.
     */
    public void checkNinbox() {
        System.out.println("You have " + ninbox + " in inbox");;
    }

    /**
     * Change username if it is unique.
     * 
     * @param uname A unique username.
     * @param accs  the accounts array that exist.
     */
    public void changeUName(String uname, Account[] accs) {
        for (int i = 0; i < usercount; i++) {
            if (accs[i].getUsername().equals(uname)) {
                return;
            }
        }
        this.username = uname;
    }

    /**
     * Login , if no other account is currently logged in.
     * 
     * @param accounts All the user accounts.
     * @return true if the login is successful, false otherwise.
     */
    public boolean login(Account[] accounts) {
        for (int i = 0; i < usercount; i++) {
            Account acc = accounts[i];
            if (acc.loggedin) {
                System.out.println("Login failed to " + this.getUsername());
                return false;
            }
        }
        this.loggedin = true;
        System.out.println("Login succesfull to " + this.getUsername());
        return true;
    }

    /**
     * Logs out the user by setting the loggedin variable to false.
     * 
     * @return true if logged in succesfull, false otherwise
     */
    public Boolean logout() {
        if (this.loggedin){
            System.out.println("User not logged in");
            loggedin = false;
        }
        
        System.out.println("Log out succesfully to " + this.getUsername());
        return true;
    }

    /**
     * Follow a user checks if already following
     *
     * @param otherUser The account of the user to follow.
     * @return true if the user successfully, false otherwise
     */
    public boolean follow(Account otherUser) {
        if (loggedin && otherUser != null) {
            // Check if this user is already following
            for (int i = 0; i < nfollowing; i++) {
                if (following[i].equals(otherUser.getID())) {
                    return false;
                }
            }
            following[nfollowing] = otherUser.getID();
            nfollowing++;
            otherUser.addFollower(this.getID());
            return true;
        } else {
            return false;
        }
    }

    /**
     * Adds the account ID of a follower to this user's followers list.
     *
     * @param followerId The account ID of the follower to add.
     */
    private void addFollower(int followerId) {
        followers[nfollowers] = followerId;
        nfollowers++;
    }

    /**
     * Send message to another user
     *
     * @param receiver the message reciever
     * @param message the message text
     * @return false if user not logged in
     */
    public Boolean sendMsg(Account receiver, String message) {
        if (!loggedin) {
            System.out.println("Please logged in before doing an operation");
            return false;
        }
        if (!contains(following, nfollowing, receiver.getID()) || contains(receiver.getBlocked(), receiver.getNBlocked(), this.getID())) {
            System.out.println("Please follow the user before send a message (clue: maybe blocked)");
            return false;
        }

        Message msg = new Message(this, receiver, message);
        outbox[noutbox] = msg;
        noutbox++;
        receiver.receiveMsg(msg);
        System.out.println("Message posted to " + receiver.getUsername());

        return true;
    }

    /**
     * Checks if an Integer array contains the element 
     *
     * @param value element to be checked
     * @return true if it contains, false otherwise
     */
    private boolean contains(Integer[] array, int size, int value) {
        for (Integer i : array) {
            if (i != null && i.intValue() == value) {
                return true;
            }
        }
        return false;
    }

    /**
     * Adds a message to the user's inbox.s
     * 
     * @param msg The message to add.
     */
    private void receiveMsg(Message msg) {
        inbox[ninbox] = msg;
        ninbox++;
    }

    /**
     * Shares a new post.
     *
     * @param content The content of the post.
     */
    public void sharePost(String content) {
        if(!loggedin) {
            System.out.println("Please logged in before doing an operation");
            return;
        }
        Post newPost = new Post(this.accountId, content);
        this.posts[this.nposts] = newPost;
        this.nposts++;
    }

    /**
     * Displays the profile information of the specified account.
     * 
     * @param user The account whose profile information will be displayed.
     * @param accs account array that exists
     */
    public void viewProfile(Account user, Account[] accs) {
        if (loggedin && user != null && !contains(user.getBlocked(), user.getNBlocked(), this.getID())) {
            viewing = user;
            System.out.println("User ID: " + user.getID());
            System.out.println("Username: " + user.getUsername());
            System.out.println("Location: " + user.getLocation());
            System.out.println("Birthdate: " + user.getBirthdate());
            System.out.println(this.getUsername() + " is following " + user.getNFollowing() + "account(s) " + "and has " + user.getNFollowers() + " follower(s)");
            System.out.println("Followers: ");
            for (int i = 0; i < getNFollowers(); i++) {
                var follower = getFollowers()[i];
                System.out.println("- " + accs[follower - 1].getUsername());
            }
            System.out.println("Following: ");
            for (int i = 0; i < getNFollowing(); i++) {
                var followin = getfollowing()[i];
                System.out.println("- " + accs[followin - 1].getUsername());
            }
            System.out.println(this.getUsername() + " has " + user.getNPosts() + " post(s)");
        } 
        else {
            System.out.println("You must be logged in to view profiles (clue: maybe blocked).");
        }
    }

    /**
     * Displays the posts of the user.
     * @param user The account whose posts will be viewed.
     */
    public void viewPosts(Account user) {
        if (loggedin && user != null && viewing.equals(user)) {
            System.out.println("Viewing " + user.getUsername() + "'s posts...");
            for (int i = 0; i < user.getNPosts(); i++) {
                Post post = user.getPost(i);
                System.out.println("(PostID: " + post.getId() + ") " + user.getUsername() + ": " + post.getContent());
            }
        } 
        else {
            System.out.println("You must be logged in to view posts.");
        }
    }

    /**
     * Likes the specified post.
     *
     * @param post The post to like.
     */
    public void like(Post post) {
        if(post == null) {
            System.out.println("Ivalid post");
            return;
        }
        if (loggedin) {
            // Check if the post has already been liked
            for (int i = 0; i < post.getNlike(); i++) {
                if (post.getLike(i).getAccountId() == this.getID()) {
                    System.out.println("You have already liked this post.");
                    return; 
                }
            }
            
            // Like the post
            Like newLike = new Like(this.getID(), post.getId());
            post.addLike(newLike);
            System.out.println("Post liked successfully.");
        } else {
            System.out.println("You must be logged in to like posts.");
        }
    }

    /**
     * Adds a comment to the given post with the given content.
     * 
     * @param post The post which will be commented.
     * @param content The content of the comment.
     */
    public void comment(Post post, String content) {
        if (loggedin && post != null && !contains(blocked, nblocked, this.getID())) {
            Comment comment = new Comment(this.getID(), post.getId(), content);
            post.addComment(comment);
            System.out.println("Comment added.");
        } else {
            System.out.println("You must be logged in to add comments.");
        }
    }

    /**
     * Blocks the specified account if the current user not already following the specified account.
     * @param user The account to block.
     */
    public void block(Account user) {
        if (loggedin && user != null) {
            if(!contains(blocked, nblocked, user.getID())){
                blocked[nblocked] = Integer.valueOf(user.getID());
                nblocked++;
                System.out.println("You have blocked " + user.getUsername() + ".");
            }
        } else {
            System.out.println("You must be logged in to block accounts.");
        }
    }

    /**
     * Displays the user's inbox.
     * 
     * @param accs account array that exists
     */
    public void viewInbox(Account[] accs) {
        // Check if the user is logged in before allowing them to view their inbox
        if (loggedin) {
            System.out.println("Inbox:");
            for (int i = 0; i < ninbox; i++) {
                Message message = inbox[i];
                System.out.println(accs[message.getSenderId() - 1].getUsername() + ": " + message.getContent());
            }
        } else {
            System.out.println("You must be logged in to view your inbox.");
        }
    }
    
    /**
     * Displays the user's outbox.
     * 
     * @param accs account array that exists
     */
    public void viewOutbox(Account[] accs) {
        // Check if the user is logged in before allowing them to view their Outbox
        if (loggedin) {
            System.out.println("Outbox:");
            for (int i = 0; i < noutbox; i++) {
                Message message = outbox[i];
                System.out.println(accs[message.getSenderId()].getUsername() + ": " + message.getContent());
            }
        } else {
            System.out.println("You must be logged in to view your Outbox.");
        }
    }

    /**
     * Views the interactions on a given post.
     *
     * @param post the post to view interactions for
     * @param accs account array that exists
     */
    public void viewPostInteractions(Post post, Account[] accs) {
        if (!loggedin) {
            System.out.println("Please log in to view post interactions.");
            return;
        }

        if (post == null) {
            System.out.println("Invalid post.");
            return;
        }

        System.out.println("Interactions for post " + post.getId() + ":");
        post.showLikes(accs);


        post.showComments(accs);

    
    }
    

}