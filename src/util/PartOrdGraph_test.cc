/*  PartOrdGraph_test.cc

    Good gtest getting started documentation:
        
    http://www.ibm.com/developerworks/aix/library/au-googletestingframework.html

    Usage if compiling on its own with the gtest-1.7.0/ dir around:
        Uncomment the main function at the bottom.
        g++ -isystem gtest-1.7.0/include -Igtest-1.7.0 -pthread -c gtest-1.7.0/src/gtest-all.cc
        ar -rv libgtest.a gtest-all.o
        g++ -isystem gtest-1.7.0/include -pthread PartOrdGraph_test.cc PartOrdGraph.cc libgtest.a -o PartOrdGraph_test

    Copyright (c) 2015, University of Arizona
    Michelle Strout
*/

#include "gtest/gtest.h"
#include "PartOrdGraph.h"

TEST(PartOrdGraphTest, Strict) {
    // Construct partial ordering an put 2->1 and 1->4 and 4->3 in
    PartOrdGraph g(5);
    g.strict(2,1);
    g.strict(1,4);
    g.strict(4,3);
    
    // Test the explicit stricts were put in there.
    EXPECT_EQ( true, g.isStrict(2,1));
    EXPECT_EQ( true, g.isStrict(1,4));
    EXPECT_EQ( true, g.isStrict(4,3));
    
    // Test that transitive closure happened.
    EXPECT_EQ( true, g.isStrict(2,4));
    EXPECT_EQ( true, g.isStrict(2,3));
    EXPECT_EQ( true, g.isStrict(1,3));
    
    // Also test some of the others.
    EXPECT_EQ( true, g.isNoOrder(4,2));
    EXPECT_EQ( true, g.isNoOrder(2,0));
    EXPECT_EQ( false, g.isNonStrict(2,1));
    EXPECT_EQ( false, g.isNonStrict(0,1));
    EXPECT_EQ( true, g.isEqual(0,0));
    EXPECT_EQ( false, g.isEqual(0,1));
}

TEST (PartOrdGraphTest, NonStrict) { 
    PartOrdGraph g(5);
    g.nonStrict(0,1);
    g.nonStrict(1,2);
    g.nonStrict(2,3);
    g.nonStrict(3,4);
    
    // Test the explicit stricts were put in there.
    EXPECT_EQ( true, g.isNonStrict(0,1));
    EXPECT_EQ( true, g.isNonStrict(1,2));
    EXPECT_EQ( true, g.isNonStrict(2,3));
    EXPECT_EQ( true, g.isNonStrict(3,4));
    
    // Test that transitive closure happened.
    EXPECT_EQ( true, g.isNonStrict(2,4));
    EXPECT_EQ( true, g.isNonStrict(0,4));
    
    // Also test some of the others.
    EXPECT_EQ( true, g.isNoOrder(4,2));
    EXPECT_EQ( true, g.isNoOrder(2,0));
    EXPECT_EQ( false, g.isNonStrict(2,1));
    EXPECT_EQ( false, g.isStrict(0,1));
    EXPECT_EQ( false, g.isEqual(1,2));
    EXPECT_EQ( false, g.isEqual(0,1));
}

TEST (PartOrdGraphTest, AllThree) { 
    PartOrdGraph g(6);
    
    // f < g, g <= foo, foo=baz
    // 0:g, 1:foo, 2:f, 3:baz
    g.strict(2,0);      // f<g
    g.nonStrict(0,1);   // g<=foo
    g.equal(1,3);       // foo=baz

    // Test that transitive closure happened.
    EXPECT_EQ( true, g.isStrict(2,1));      // f<foo
    EXPECT_EQ( true, g.isStrict(2,3));      // f<baz
    EXPECT_EQ( true, g.isNonStrict(0,1));   // g<=baz
  
    // create an equality
    g.nonStrict(1,0);                       // baz<=g
    EXPECT_EQ( true, g.isEqual(0,1) );
}

/* MMS: do not delete.  See usage directions in header.
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
*/
