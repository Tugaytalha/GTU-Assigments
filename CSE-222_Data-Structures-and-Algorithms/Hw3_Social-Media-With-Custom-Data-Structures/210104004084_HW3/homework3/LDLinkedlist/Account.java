package homework3.LDLinkedList;

import java.util.Iterator;

/**
 * This class represents a user social media account account and it's utility
 * functions.
 */
public class Account {

    private Integer accountId;
    private String username;
    private String birthdate;
    private String location;
    private LDLinkedList<Post> posts;
    private LDLinkedList<Message> inbox;
    private LDLinkedList<Message> outbox;
    private LDLinkedList<Integer> following;
    private LDLinkedList<Integer> followers;
    private LDLinkedList<Integer> blocked;
    private LDLinkedList<String> history;
    private Account viewing = this;
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
        history = new LDLinkedList<String>();
        changeUName(uname, accs);
        posts = new LDLinkedList<Post>();
        inbox = new LDLinkedList<Message>();
        outbox = new LDLinkedList<Message>();
        following = new LDLinkedList<Integer>();
        followers = new LDLinkedList<Integer>();
        blocked = new LDLinkedList<Integer>();
        usercount++;
        this.accountId = Integer.valueOf(usercount);
        this.birthdate = bdate;
        this.location = loc;
        this.loggedin = false;
        accs[usercount - 1] = this;
        if(username != "take_unique_username") {
            System.out.println("Account created succesfully");
        }
        else {
            System.out.println("Account created but you need new user name.");
        }
    }

    /**
     * Returns followers array.
     * 
     * @return An array of follower account IDs.
     */
    public LDLinkedList<Integer> getFollowers() {
        return followers;
    }

    /**
     * returns the blocked array 
     *
     * @return The blocked array.
     */
    public LDLinkedList<Integer> getBlocked() {
        return blocked;
    }

    /**
     * Getter for number of blocked users.
     *
     * @return The number of blocked users.
     */
    public int getNBlocked() {
        return blocked.size();
    }


    /**
     * Returns the following array.
     * 
     * @return An array of following account IDs.
     */
    public LDLinkedList<Integer> getfollowing() {
        return following;
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
     * Returns the number of users (includes lazy deleted) who follow this account.
     * 
     * @return The number of followers.
     */
    public int getNFollowers() {
        return followers.size();
    }

    /**
     * Returns the number of users (includes lazy deleted) this account is following.
     * 
     * @return The number of accounts being followed.
     */
    public int getNFollowing() {
        return following.size();
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
        return posts.size();
    }

    /**
     * Return the post that have given index.
     *
     * @param i index for posts array.
     * @return Post at given index.
     */
    public Post getPost(int i) {
        return posts.get(i);
    }
    
    /**
     * Print the number of msg in outbox.
     *
     */
    public void checkNoutbox() {
        System.out.println("You have " + outbox.ndsize() + " in outbox");;
    }

    /**
     * Print the number of msg in inbox.
     */
    public void checkNinbox() {
        System.out.println("You have " + inbox.ndsize() + " in inbox");;
    }
    
    /**
     * Add a action to history.
     * 
     * @param item String to be added
     */
    public void addHistory(String item) {
        history.add(item);
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
        System.out.println("User name has changed.");
        this.addHistory("User name changed to" + uname);
    }

    /**
     * Login , if no other account is currently logged in.
     * 
     * @param accounts All the user accounts.
     * @return true if the login is successful, false otherwise.
     */
    public boolean login(Account[] accounts) {
        System.out.println();
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
        System.out.println();
        if (!this.loggedin){
            System.out.println("User not logged in");
            return false;
        }
        
        loggedin = false;
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
        System.out.println();
        if (loggedin && otherUser != null) {
            // Check if this user is already following
            for (int i = 0; i < following.size(); i++) {
                var followi = following.get(i);
                if (followi != null && followi.equals(otherUser.getID())) {
                    return false;
                }
            }
            this.following.add(otherUser.getID());
            this.addHistory("You followed " + otherUser.getUsername());
            otherUser.addFollower(this.getID());
            otherUser.addHistory(otherUser.getUsername() + "started following you");
            System.out.println("Succesfully followed");
            return true;
        } else {
            return false;
        }
    }

    /**
     * Unfollows the specified account.
     *
     * @param otherUser The account to unfollow.
     * @return True if the unfollowing is successful, false otherwise.
     */
    public boolean unfollow(Account otherUser) {
        System.out.println();
        if (loggedin && otherUser != null) {
            // Check if this user is already following
            if (following.contains(otherUser.getID())) {
                following.remove(following.find(otherUser.getID()));
                this.addHistory("You unfollowed " + otherUser.getUsername());
                otherUser.removeFollower(this.getID());
                otherUser.addHistory(otherUser.getUsername() + "stopped following you");
                System.out.println("Successfully unfollowed");
                return true;
            } else {
                System.out.println("You are not following this user");
                return false;
            }
        } else {
            System.out.println("Login before unfollow.");
            return false;
        }
    }

    /**
     * private, no login version of unfollow
     * Unfollows the specified account.
     *
     * @param otherUser The account to unfollow.
     * @return True if the unfollowing is successful, false otherwise.
     */
    private boolean punfollow(Account otherUser) {
        if (otherUser != null) {
            // Check if this user is already following
            if (following.contains(otherUser.getID())) {
                following.remove(following.find(otherUser.getID()));
                this.addHistory("You unfollowed " + otherUser.getUsername());
                otherUser.removeFollower(this.getID());
                otherUser.addHistory(otherUser.getUsername() + "stopped following you");
                return true;
            } else {
                return false;
            }
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
        followers.add(followerId);
    }

    /**
     * Removes the follower with the given account ID.
     *
     * @param accountId The ID of the follower to be removed.
     * @return true if the follower removed, false otherwise.
     */
    public boolean removeFollower(int accountId) {
        boolean flag = false;
        for (int i = 0; i < followers.size(); i++) {
            if (followers.get(i) == accountId) {
                followers.remove(i);
                flag = true;
                break;
            }
        }
        return flag;
    }

    /**
     * Send message to another user
     *
     * @param receiver the message reciever
     * @param message the message text
     * @return false if user not logged in
     */
    public Boolean sendMsg(Account receiver, String message) {
        System.out.println();
        if (!loggedin) {
            System.out.println("Please logged in before doing an operation");
            return false;
        }
        if (!contains(following, receiver.getID()) || contains(receiver.getBlocked(), this.getID())) {
            System.out.println("Please follow the user before send a message (clue: maybe blocked)");
            return false;
        }

        Message msg = new Message(this, receiver, message);
        outbox.add(msg);
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
    private boolean contains(LDLinkedList<Integer> array, int value) {
        for (var i : array) {
            if (i != null && i == value) {
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
        inbox.add(msg);
    }

    /**
     * Shares a new post.
     *
     * @param content The content of the post.
     */
    public void sharePost(String content) {
        System.out.println();
        if(!loggedin) {
            System.out.println("Please logged in before doing an operation");
            return;
        }
        Post newPost = new Post(this.accountId, content);
        this.posts.add(newPost);
        System.out.println("Posted succesfully.");
        this.addHistory("You posted: ID:" + newPost.getId() + " Content: " + newPost.getContent());
    }

    /**
     * Displays the profile information of the specified account.
     * 
     * @param user The account whose profile information will be displayed.
     * @param accs account array that exists
     */
    public void viewProfile(Account user, Account[] accs) {
        System.out.println();
        if (loggedin && user != null && !contains(user.getBlocked(), this.getID())) {
            viewing = user;
            System.out.println("User ID: " + user.getID());
            System.out.println("Username: " + user.getUsername());
            System.out.println("Location: " + user.getLocation());
            System.out.println("Birthdate: " + user.getBirthdate());
            System.out.println(user.getUsername() + " is following " + user.following.ndsize() + " account(s) " + "and has " + user.followers.ndsize() + " follower(s)");
            System.out.println("Following: ");
            for (int i = 0; i < user.getNFollowing(); i++) {
                Integer followin = user.getfollowing().get(i);
                if(followin != null)
                System.out.println("- " + accs[followin - 1].getUsername());
            }
            System.out.println("Followers: ");
            for (int i = 0; i < user.getNFollowers(); i++) {
                Integer follower = user.getFollowers().get(i);
                if(follower != null)
                System.out.println("- " + accs[follower - 1].getUsername());
            }
            System.out.println(user.getUsername() + " has " + user.getNPosts() + " post(s)");
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
        System.out.println();
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
        System.out.println();
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
            
            Like newLike = new Like(this.getID(), post.getId());
            post.addLike(newLike);
            System.out.println("Post liked successfully.");
            this.addHistory("You liked a post ID:" + newLike.getPostId());
        } else {
            System.out.println("You must be logged in to like posts.");
        }
    }

    /**
     * Removes a like from the specified post.
     *
     * @param post The post to remove the like from.
     */
    public void unlike(Post post) {
        System.out.println();
        if(post == null) {
            System.out.println("Invalid post");
            return;
        }
        if (loggedin) {
            boolean likeFound = false;
            for (int i = 0; i < post.getNlike(); i++) {
                if (post.getLike(i).getAccountId() == this.getID()) {
                    post.removeLike(i);
                    System.out.println("Post unliked successfully.");
                    this.addHistory("You unliked a post ID:" + post.getId());
                    likeFound = true;
                    break;
                }
            }
            if (!likeFound) {
                System.out.println("You have not liked this post.");
            }
        } else {
            System.out.println("You must be logged in to unlike posts.");
        }
    }

    /**
     * Adds a comment to the given post with the given content.
     * 
     * @param post The post which will be commented.
     * @param content The content of the comment.
     */
    public void comment(Post post, String content) {
        System.out.println();
        if (loggedin && post != null && !contains(blocked, this.getID())) {
            Comment comment = new Comment(this.getID(), post.getId(), content);
            post.addComment(comment);
            System.out.println("Comment added.");
            this.addHistory("You commented" + comment.getContent() + "to post ID:" + comment.getPostId());
        } else {
            System.out.println("You must be logged in to add comments.");
        }
    }

    /**
     * Removes a comment with the given content from the given post.
     *
     * @param post The post from which the comment will be removed.
     * @param content The content of the comment to be removed.
     */
    public void uncomment(Post post, String content) {
        System.out.println();
        if (loggedin && post != null && !contains(blocked, this.getID())) {
            boolean commentFound = false;
            for (int i = 0; i < post.getNcomment(); i++) {
                var comment = post.getComment(i);
                if (comment.getContent().equals(content)) {
                    if (comment.getAccountId() == this.getID()) {
                        post.removeComment(comment);
                        System.out.println("Comment removed.");
                        this.addHistory("You removed a comment with content '" + content + "' from post ID:" + comment.getPostId());
                    } else {
                        System.out.println("You cannot remove comments made by other users.");
                    }
                    commentFound = true;
                    break;
                }
            }
            if (!commentFound) {
                System.out.println("No comment found with content '" + content + "'.");
            }
        } else {
            System.out.println("You must be logged in to remove comments.");
        }
    }


    /**
     * Blocks the specified account if the current user not already following the specified account.
     * @param user The account to block.
     */
    public void block(Account user) {
        System.out.println();
        if (loggedin && user != null) {
            if(!contains(blocked, user.getID())){
                blocked.add(Integer.valueOf(user.getID()));
                this.unfollow(user);
                user.punfollow(this);
                System.out.println("You have blocked " + user.getUsername() + ".");
                this.addHistory("You have blocked " + user.getUsername() + ".");
            }
        } else {
            System.out.println("You must be logged in to block accounts.");
        }
    }

    /**
     * Unblocks the specified account.
     * @param user The account to unblock.
     */
    public void unblock(Account user) {
        System.out.println();
        if (loggedin && user != null) {
            if (contains(blocked, user.getID())) {
                blocked.remove(Integer.valueOf(user.getID()));
                System.out.println("You have unblocked " + user.getUsername() + ".");
                this.addHistory("You have unblocked " + user.getUsername() + ".");
            } else {
                System.out.println("You haven't blocked this user.");
            }
        } else {
            System.out.println("You must be logged in to unblock accounts.");
        }
    }

    /**
     * Displays the user's activity history.
     */
    public void showHistory() {
        System.out.println();
        System.out.println("Activity history for " + this.getUsername() + ":");

        Iterator<String> historyIterator = history.iterator();

        while (historyIterator.hasNext()) {
            String activity = historyIterator.next();
            System.out.println(activity);
        }
    }

    /**
     * Displays the user's inbox.
     * 
     * @param accs account array that exists
     */
    public void viewInbox(Account[] accs) {
        System.out.println();
        // Check if the user is logged in before allowing them to view their inbox
        if (loggedin) {
            System.out.println("Inbox:");
            for (int i = 0; i < inbox.size(); i++) {
                Message message = inbox.get(i);
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
        System.out.println();
        // Check if the user is logged in before allowing them to view their Outbox
        if (loggedin) {
            System.out.println("Outbox:");
            for (int i = 0; i < outbox.size(); i++) {
                Message message = outbox.get(i);
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
        System.out.println();
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