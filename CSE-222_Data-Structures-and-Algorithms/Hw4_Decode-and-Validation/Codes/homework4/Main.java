package homework4;

public class Main {
   
    public static void main(String[] args) {
        int[] denomination = {4, 17, 29};
        // variable assignments so that the variables we test are more visible and can be tested with other variables
        String trueUname = "sibel";
        String truePass1 = "[rac()ecar]";
        int truePass2 = 75;


        TestClass.test(trueUname, truePass1, truePass2, denomination);
        TestClass.test("", truePass1, truePass2, denomination);
        TestClass.test("sibel1", truePass1, truePass2, denomination);
        TestClass.test(trueUname, "passs[]", truePass2, denomination);
        TestClass.test(trueUname, "abcdabcd", truePass2, denomination);
        TestClass.test(trueUname, "[[[[]]]]", truePass2, denomination);
        TestClass.test(trueUname, "[no](no)", truePass2, denomination);
        TestClass.test(trueUname, "[rac()ecar]]", truePass2, denomination);
        TestClass.test(trueUname, "[rac()ecars]", truePass2, denomination);
        TestClass.test(trueUname, truePass1, 5, denomination);
        TestClass.test(trueUname, truePass1, 35, denomination);
    }
}
