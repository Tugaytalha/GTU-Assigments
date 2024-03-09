package homework3.ArrayList;

import java.time.Duration;

public class TestClass {
    /**
     * Test Function to social media application
     * @param args Not used
     */
    public static void main(String[] args) {

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SCENARIO 1
        System.out.println("////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// \n//SCENARIO 1\n\n");

        Account[] Accounts = new Account[30];
        long start1 = System.currentTimeMillis();

        // Step 1: Creating accounts
        Account gizemsungu = new Account("gizemsungu", "19.02.1996", "Gebze/Kocaeli", Accounts);
        Account sibelgulmez = new Account("sibelgulmez", "08.04.1995", "Tuzla/Istanbul", Accounts);
        Account gokhankaya = new Account("gokhankaya", "21.07.1990", "Inegol/Bursa", Accounts);
        
        // Step 2: Logging in to sibelgulmez's account
        sibelgulmez.login(Accounts);
        
        // Step 3: Sharing two posts
        sibelgulmez.sharePost("I like Java.");
        sibelgulmez.sharePost("Java the coffee...");
        
        // Step 4: Following gizemsungu and gokhankaya
        sibelgulmez.follow(gizemsungu);
        sibelgulmez.follow(gokhankaya);
        
        // Step 5: Logging out from sibelgulmez's account
        sibelgulmez.logout();
        
        // Step 6: Logging in to gokhankaya's account
        gokhankaya.login(Accounts);
        
        // Step 7: Viewing sibelgulmez's profile
        gokhankaya.viewProfile(sibelgulmez, Accounts);
        
        // Step 8: Viewing sibelgulmez's posts
        gokhankaya.viewPosts(sibelgulmez);
        
        // Step 9: Liking a post of sibelgulmez
        gokhankaya.like(sibelgulmez.getPost(1));
        
        // Step 10: Commenting on a post of sibelgulmez
        gokhankaya.comment(sibelgulmez.getPost(1), "me too!");
        
        // Step 11: Following sibelgulmez and gizemsungu
        gokhankaya.follow(sibelgulmez);
        gokhankaya.follow(gizemsungu);
        
        // Step 12: Sending a message to gizemsungu
        gokhankaya.sendMsg(gizemsungu, "This homework is too easy!\n I was just kidding xD");
        
        // Step 13: Logging out from gokhankaya's account
        gokhankaya.logout();
        
        // Step 14: Logging in to gizemsungu's account
        gizemsungu.login(Accounts);
        
        // Step 15: Checking the outbox
        gizemsungu.checkNoutbox();
        
        // Step 16: Checking the inbox
        gizemsungu.checkNinbox();
        
        // Step 17: Viewing the messages in the inbox
        gizemsungu.viewInbox(Accounts);
        
        // Step 18: Viewing sibelgulmez's profile
        gizemsungu.viewProfile(sibelgulmez, Accounts);
        
        // Step 19: Viewing sibelgulmez's posts
        gizemsungu.viewPosts(sibelgulmez);
        
        // Step 20: Viewing sibelgulmez's posts' interactions
        gizemsungu.viewPostInteractions(sibelgulmez.getPost(0), Accounts);
        gizemsungu.viewPostInteractions(sibelgulmez.getPost(1), Accounts);
        
        // Step 21: Liking sibelgulmez's posts
        gizemsungu.like(sibelgulmez.getPost(0));
        gizemsungu.like(sibelgulmez.getPost(1));
        
        // Step 22: Viewing sibelgulmez's posts' interactions
        gizemsungu.viewPostInteractions(sibelgulmez.getPost(0), Accounts);
        gizemsungu.viewPostInteractions(sibelgulmez.getPost(1), Accounts);

        // Step 23: Logging out from gizemsungu's account
        gizemsungu.logout();

        long end1 = System.currentTimeMillis(); 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SCENARIO 2
        System.out.println("////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// \n//SCENARIO 2\n\n");
        long start2 = System.currentTimeMillis();

        // Step 1: Logging in to gizemsungu's account
        gizemsungu.login(Accounts);
        
        // Step 1.a & 1.b:  Sharing two posts.
        gizemsungu.sharePost("Limited time");
        gizemsungu.sharePost("I have to do this homework before deadline");
        
        // Step 1.c: Logging out from gizemsungu's account
        gizemsungu.logout();
        
        // Step 2: Logging in to sibelgulmez's account
        sibelgulmez.login(Accounts);
        
        // Step 2.a & 2.b: View gizemsungu's profile & like post3.
        sibelgulmez.viewProfile(gizemsungu, Accounts);
        try {
            sibelgulmez.like(gizemsungu.getPost(2));
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
        

        // Step 2.c: Logging out from sibelgulmez's account
        sibelgulmez.logout();
        
        // Step 3: Logging in to gokhankaya's account
        gokhankaya.login(Accounts);
        
        // Step 3.a: Viewing gizemsungu's profile
        gokhankaya.viewProfile(gizemsungu, Accounts);
        
        // Step 3.b: Commenting on gizemsungu's post
        gokhankaya.comment(gizemsungu.getPost(1), "Nice!");
        
        // Step 3.c: Sending a message to gizemsungu
        gokhankaya.sendMsg(gizemsungu, "Hello!");
        
        // Step 3.d: Logging out from gokhankaya's account
        gokhankaya.logout();
        
        // Step 4: Logging in to gizemsungu's account
        gizemsungu.login(Accounts);
        
        // Step 4.a & 4.b: Viewing her own profile
        gizemsungu.viewProfile(gizemsungu, Accounts);
        gizemsungu.viewInbox(Accounts);
                
        // Step 17: Logging out from gizemsungu's account
        gizemsungu.logout();

        long end2 = System.currentTimeMillis();  

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SCENARIO 3
        System.out.println("////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// \n//SCENARIO 3\n\n");
        long start3 = System.currentTimeMillis();


        // Step 2: Logging in to gizemsungu's account
        gizemsungu.login(Accounts);

        // Step 3: Blocking sibelgulmez
        gizemsungu.block(sibelgulmez);

        // Step 4: Logging out from gizemsungu's account
        gizemsungu.logout();

        // Step 5: Logging in to sibelgulmez's account
        sibelgulmez.login(Accounts);

        // Step 6: Trying to view gizemsungu's profile (blocked)
        sibelgulmez.viewProfile(gizemsungu, Accounts);

        // Step 7: Trying to send a message to gizemsungu (blocked)
        sibelgulmez.sendMsg(gizemsungu, "Hi!");

        // Step 8: Logging out from sibelgulmez's account
        sibelgulmez.logout();

        long end3 = System.currentTimeMillis();  

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SCENARIO 4
        System.out.println("////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// \n//SCENARIO 4\n\n");
        long start4 = System.currentTimeMillis();

        // Step 1: Logging in to gizemsungu's account & following gokhankaya
        gizemsungu.login(Accounts);
        gizemsungu.follow(gokhankaya);

        // Step 2: View gizemsungu's profile
        gizemsungu.viewProfile(gizemsungu, Accounts);
        
        // Step 3: Unblocking sibelgulmez
        gizemsungu.unblock(sibelgulmez);
        
        // Step 4.a & 4.b: Unfollowing gokhankaya and following sibelgulmez
        gizemsungu.unfollow(gokhankaya);
        gizemsungu.follow(sibelgulmez);
        
        // Step 5: View gizemsungu's profile to see changes 
        gizemsungu.viewProfile(gizemsungu, Accounts);
        
        // Step 6: Logging out from gizemsungu's account
        gizemsungu.logout();

        // Step 7: Logging in to gokhankaya's account
        gokhankaya.login(Accounts);

        // Step 8: View sibelgulmez's profile 
        gokhankaya.viewProfile(sibelgulmez, Accounts);
        
        // Step 9.a & 9.b: Unlike & uncomment post2 and show it's happened
        gokhankaya.viewPostInteractions(sibelgulmez.getPost(1), Accounts);
        gokhankaya.unlike(sibelgulmez.getPost(1));
        gokhankaya.uncomment(sibelgulmez.getPost(1), "me too!");
        gokhankaya.viewPostInteractions(sibelgulmez.getPost(1), Accounts);
        
        // Step 10: Showing activity history for gokhankaya 
        gokhankaya.showHistory();
        
        // Step 11: Logging out from gokhankaya's account
        gokhankaya.logout();

        long end4 = System.currentTimeMillis();  

        System.out.println("Scenario 1:" + (end1-start1) + "ms");
        System.out.println("Scenario 2:" + (end2-start2) + "ms");
        System.out.println("Scenario 3:" + (end3-start3) + "ms");
        System.out.println("Scenario 4:" + (end4-start4) + "ms");
    }
}