package homework1;

/**
 * This class represents a social media post.
 */
public class Post {

    private int postId;
    private int accountId;
    private Like[] likes = new Like[100];
    private int nlike;
    private Comment[] comments = new Comment[100];
    private int ncomment;
    private String content;
    private static int postCount = 0;
 
    /**
     * Constructs that takes account ID and content.
     * @param accountId The account ID that created the post.
     * @param content The content of the post.
     */
    public Post(int accountId, String content) {
        postCount++;
        ncomment = 0;
        nlike = 0;
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
        likes[nlike] = like;
        nlike++;
        return like;
    }

    /**
     * Adds a comment to the post.
     *
     * @param comment comment to be added
     * @return the new comment that was added
     */
    public Comment addComment(Comment comment) {
        comments[ncomment] = comment;
        ncomment++;
        return comment;
    }

    /**
     * Getter for the number of comments
     *
     * @return The number of comments.
     */
    public int getNcomment() {
        return ncomment;
    }

    /**
     * Getter for the number of likes.
     *
     * @return The number of likes.
     */
    public int getNlike() {
        return nlike;
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
        return likes[index];
    }

    /**
     * Returns the comment at the specified index.
     *
     * @param index The index of the comment.
     * @return The comment object at the specified index.
     */
    public Comment getComment(int index) {
        return comments[index];
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
        if (this.getNlike() == 0) {
            System.out.println("No one has liked this post yet.");
        } else {
            System.out.println("Likes:");
            for (int i = 0; i < this.getNlike(); i++) {
                System.out.println("- " + accs[this.getLike(i).getAccountId()].getUsername());
            }
        }
    }

    /**
     * Shows the comments on this post.
     * @param accs account array that exists
     */
    public void showComments(Account[] accs) {
        if (this.getNcomment() == 0) {
            System.out.println("No comments on this post yet.");
        } else {
            System.out.println("Comments:");
            for (int i = 0; i < this.getNcomment(); i++) {
                System.out.println(accs[this.getComment(i).getAccountId()].getUsername() + ": " + this.getComment(i).getContent());
            }
        }
    }

}


