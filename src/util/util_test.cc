/*!
 * \file util_test.cc
 *
 * \brief Tests for the IEGen utilities
 *
 * \date Started: 9/13/10
 * # $Revision:: 622                $: last committed revision
 * # $Date:: 2013-01-18 13:11:32 -0#$: date of last committed revision
 * # $Author:: cathie               $: author of last committed revision
 *
 * \authors Alan LaMielle
 *
 * Copyright (c) 2009, 2010, 2011, 2012, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "util.h"
#include <iegenlib.h>
#include <gtest/gtest.h>

using iegenlib::replaceInString;
using iegenlib::parse_exception;
using iegenlib::assert_exception;

TEST(UtilTests, ReplaceInString) {
    EXPECT_EQ("replacement",
              replaceInString("target", "target", "replacement"));
    EXPECT_EQ("hello replaced world replaced!",
              replaceInString("hello replaceme world replaceme!", "replaceme",
                              "replaced"));
    EXPECT_EQ("bb", replaceInString("aa", "a", "b"));
    EXPECT_EQ("hello hello world", replaceInString("hello world", "world", "hello world"));
    EXPECT_EQ("unchanged", replaceInString("unchanged", "other", "new"));
    EXPECT_EQ("", replaceInString("", "", "a"));
    EXPECT_EQ("", replaceInString("eraseme", "eraseme", ""));
}

TEST(UtilTests, GenerateDotLabel) {
   EXPECT_EQ("label=\"\\{ \\[0\\] \\}\n double r = current_values\\[0\\], theta=current_values\\[1\\], phi=current_values\\[2\\];\"",
             generateDotLabel("{ [0] }\n double r = current_values[0], theta=current_values[1], phi=current_values[2];"));
   EXPECT_EQ("label=\"\\{ \\[100\\] \\}\n double _iegen_17_iegen_16x = _iegen_17r_eval;\"",
             generateDotLabel("{ [100] }\n double $_iegen_17_iegen_16x$ = $_iegen_17r_eval$;"));
   EXPECT_EQ("label=\"\\{ \\[0\\] \\}\\n double r = current_values\\[0\\], theta=current_values\\[1\\], phi=current_values\\[2\\];\"",
             generateDotLabel({"{ [0] }", "\\n ", "double r = current_values[0], theta=current_values[1], phi=current_values[2];"}));
}

// TODO: Test the StringException and ParseException classes and any other util classes/routines
TEST(UtilTests, UtilTestParse){

  try{
    parse_exception excep = parse_exception("test");
    throw excep;
  }catch(parse_exception e){
     string in = e.what();
     EXPECT_EQ(in,"test");
     return; //Success!
  }
  ADD_FAILURE()<<"ParseException exception not raised!";
}

TEST(UtilTests, UtilTestAssert){

  try{
    assert_exception excep = assert_exception("test");
    throw excep;
  }catch(assert_exception e){
     string in = e.what();
     EXPECT_EQ(in,"test");
     return; //Success!
  }
  ADD_FAILURE()<<"assert_exception exception not raised!";
}

