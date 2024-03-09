package homework3.LinkedList;

import java.util.Iterator;
import java.util.LinkedList;


/**
 * This class represents a social media post.
 */
public class Post {

    private int postId;
    private int accountId;
    private LinkedList<Like> likes = new LinkedList<Like>();
    private LinkedList<Comment> comments = new LinkedList<Comment>();
    private String content;
    private static int postCount = 0;
 
    /**
     * Constructs that takes account ID and content.
     * @param accountId The account ID that created the post.
     * @param content The content of the post.
     */
    public Post(int accountId, String content) {
        postCount++;
        this.postId = postCount;
        this.accountId = accountId;
        this.content = content;
    }

    /**
     * Adds a like to the post.
     *
     * @param like like to be added 
     * @return the new like that added
     */
    public Like addLike(Like like) {
        likes.add(like);
        return like;
    }

    /**
     * Removes the like at the specified index from the post's list of likes.
     *
     * @param index The index of the like to remove.
     * @return The like that was removed.
     */
    public Like removeLike(int index) {
        Like removedLike = likes.remove(index);
        return removedLike;
    }

    /**
     * Adds a comment to the post.
     *
     * @param comment comment to be added
     * @return the new comment that was added
     */
    public Comment addComment(Comment comment) {
        comments.add(comment);
        return comment;
    }

    /**
     * Getter for the number of comments
     *
     * @return The number of comments.
     */
    public int getNcomment() {
        return comments.size();
    }

    /**
     * Getter for the number of likes.
     *
     * @return The number of likes.
     */
    public int getNlike() {
        return likes.size();
    }

    /**
     * Return ID.
     *
     * @return Id of the post.
     */
    public int getId() {
        return postId;
    }

    /**
     * Return content.
     *
     * @return content of the post.
     */
    public String getContent() {
        return content;
    }

    /**
     * Returns the like at the specified index.
     *
     * @param index The index of the like.
     * @return The like object at the specified index.
     */
    public Like getLike(int index) {
        return likes.get(index);
    }


    /**
     * Removes the specified comment from the post.
     *
     * @param comment The comment to remove.
     * @return True if the comment was removed, false otherwise.
     */
    public boolean removeComment(Comment comment) {
        Iterator<Comment> iterator = comments.iterator();
        int index = 0;
        while (iterator.hasNext()) {
            Comment c = iterator.next();
            if (c.equals(comment)) {
                comments.remove(index);
                System.out.println("Comment removed.");
                return true;
            }
            index++;
        }
        System.out.println("Comment not found.");
        return false;
    }

    /**
     * Returns the comment at the specified index.
     *
     * @param index The index of the comment.
     * @return The comment object at the specified index.
     */
    public Comment getComment(int index) {
        return comments.get(index);
    }


    /**
     * Returns a string representation of this post.
     */
    @Override
    public String toString() {
        return "Post #" + postId + " by account #" + accountId + ": " + content;
    }

    /**
     * Shows the users who have liked this post.
     * @param accs account array that exists
     */
    public void showLikes(Account[] accs) {
        if(likes != null){
            if (this.getNlike() == 0) {
                System.out.println("No one has liked this post yet.");
            } else {
                System.out.println("Likes:");
                for (int i = 0; i < this.getNlike(); i++) {
                    var likem = this.getLike(i);
                    if(likem != null)
                    System.out.println("- " + accs[this.getLike(i).getAccountId() - 1].getUsername());
                }
            }
        }
    }

    /**
     * Shows the comments on this post.
     * @param accs account array that exists
     */
    public void showComments(Account[] accs) {
        if(likes != null){
            if (this.getNcomment() == 0) {
                System.out.println("No comments on this post yet.");
            } else {
                System.out.println("Comments:");
                for (int i = 0; i < this.getNcomment(); i++) {
                    System.out.println(accs[this.getComment(i).getAccountId() -1].getUsername() + ": " + this.getComment(i).getContent());
                }
            }
        }
    }

}


