/*!
 * \file UFCallMap_test.cc
 *
 * \brief Test for the UFCallMap class.
 *
 * \date Started: 2016-03-14
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2015-2016, University of Arizona <br>
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */

#include "isl_str_manipulation.h"

#include <gtest/gtest.h>
#include <utility>
#include <iostream>
#include <fstream>

using namespace std;
using namespace iegenlib;

#pragma mark revertISLTupDeclToOrig
// Test the revertISLTupDeclToOrig functionality
TEST(revertISLTupDeclToOrigTest, revertISLTupDeclToOrig) {

    std::string orig(" [tstep, theta_0__tv1_, col_tv2_]->{[ i1, i2]->"
                     "[ 0, i4, i5 ,  i6] : i1 = col_i_ and i4 = tstep"
                                      " and i6 = theta_0__tv1_ + 1}");
    std::string islStr(" {[col_i_ , i2] -> [0 , tstep, i5, theta_0__tv1_ + 1] }");
    string exp(" {[ i1, i2]->[ 0, i4, i5 ,  i6] : ( i1) - (col_i_ ) = 0 and"
              " ( i4) - ( tstep) = 0 and (  i6) - ( theta_0__tv1_ + 1) = 0}");
    int inArity = 2, outArity = 4;
    string corrected;

    corrected = revertISLTupDeclToOrig(orig, islStr, inArity, outArity);

    //cout<<endl<<corrected<<endl<<endl; 
    EXPECT_EQ( exp , corrected );

    std::string s_orig(" {[ i1, i2, i3, i4] : i1 = col_i_ and i4 = tstep and i4 = theta_0__tv1_ + 1 and i1 >= 0 and i2 <= 10}");
    std::string s_islStr(" {[col_i_ , i2, tstep, theta_0__tv1_ + 1] : "
                                                          " i2 <= 10 }");
    string s_exp(" {[ i1, i2, i3, i4] :  i2 <= 10  and ( i1) - (col_i_ ) = 0"
            " and ( i3) - ( tstep) = 0 and ( i4) - ( theta_0__tv1_ + 1) = 0}");

    inArity = 4;outArity = 0;
    corrected = revertISLTupDeclToOrig(s_orig, s_islStr, inArity, outArity);

    //cout<<endl<<corrected<<endl<<endl; 
    EXPECT_EQ( s_exp , corrected );
}

#pragma mark projectOutStrCorrection
// Test the projectOutStrCorrection functionality
TEST(revertISLTupDeclToOrigTest, projectOutStrCorrection) {

    string islStr("{[col_i_ , i2] -> [0 , tstep, i5, theta_0__tv1_ + 1] }");
    string exp(   "{[col_i_ ,  i2] -> [ tstep,  i5,  theta_0__tv1_ + 1] }");
    int inArity = 2, outArity = 4;
    string corrected;

    corrected = projectOutStrCorrection(islStr, 2, inArity, outArity);

    EXPECT_EQ( exp , corrected );

    exp = "{[col_i_ ,  i2] -> [0 ,  tstep] }";
    inArity = 2; outArity = 3;

    corrected = projectOutStrCorrection(islStr, 4, inArity, outArity);

    EXPECT_EQ( exp , corrected );


    std::string s_islStr(" {[col_i_ , i2, tstep, theta_0__tv1_ + 1] : "
                                                "col_i_ >= 0 and i2 <= 10}");
    string s_exp(        " {[col_i_ ,  i2,  tstep] : "
                                                "col_i_ >= 0 and i2 <= 10}");

    inArity = 4;outArity = 0;
    corrected = projectOutStrCorrection(s_islStr, 3, inArity, outArity);

    //cout<<endl<<corrected<<endl<<endl; 
    EXPECT_EQ( s_exp , corrected );
}
