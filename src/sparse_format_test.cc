#include <gtest/gtest.h>

#include <iegenlib.h>
using namespace iegenlib;
using namespace std;

/*!
 * \class Sparse Format Tests
 *
 * \brief Class to Sparse Format compositions in.
 *
 * This class holds gtest test cases for sparse format conversions
 * and code synthesis.
 */
class SparseFormatTest : public ::testing::Test {
public:
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

// Test for COO WRT Dense
TEST_F(SparseFormatTest, COOWRTDense_1) {
  Relation *r = new Relation("{[n] -> [i,j] : row(n) = i && col(n) = j "
                             "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
  EXPECT_EQ("{ [n] -> [i, j] : i - row(n) = 0"
            " && j - col(n) = 0 && n >= 0 && Dense(i, j) - 1"
            " >= 0 && -n + NNZ - 1 >= 0 }",
            r->prettyPrintString());
  delete r;
}
// Test for CSR WRT DENSE
TEST_F(SparseFormatTest, CSRWRTDense_1) {
  Relation *r = new Relation("{[n] -> [i,j] :n >= rptr(i) "
                             "&& n < rptr(i+1) && col(n) = j "
                             "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
  // UF for rptrcsr
  EXPECT_EQ("{ [n] -> [i, j] : j - col(n) = 0 && n >= 0 && n - rptr(i) >= 0 && "
            "Dense(i, j) - 1 >= 0 && -n + NNZ - 1 >= 0 && -n + rptr(i + 1) - 1 "
            ">= 0 }",
            r->prettyPrintString());
  delete r;
}

// Test for COO TO CSR COMPOSE
TEST_F(SparseFormatTest, COO_CSR_1) {
  Relation *r_coo = new Relation("{[n] -> [i,j] : row(n) = i && col(n) = j "
                                 "&& 0 <= n && n < NNZ }");
  Relation *r_csr = new Relation("{[n] -> [i,j] :n >= rptr(i) "
                                 "&& n < rptr(i+1) && rowcol_inv(i,j) = n "
                                 "&& 0 <= n && n < NNZ }");
  // UF for rptrcsr
  // to be gotten using chill

  EXPECT_EQ("{ [n] -> [i, j] : n - rowcol_inv(i, j) = 0 && n >= 0 && n - "
            "rptr(i) >= 0 && -n + NNZ - 1 >= 0 && -n + rptr(i + 1) - 1 >= 0 }",
            r_csr->prettyPrintString());

  EXPECT_EQ("{ [n] -> [i, j] : i - row(n) = 0 && j - col(n) = 0 && n >= 0 && "
            "-n + NNZ - 1 >= 0 }",
            r_coo->prettyPrintString());
  Relation *r_csr_inv = r_csr->Inverse();
  EXPECT_EQ("{ [i, j] -> [n] : n - rowcol_inv(i, j) = 0 && n >= 0 && n - "
            "rptr(i) >= 0 && -n + NNZ - 1 >= 0 && -n + rptr(i + 1) - 1 >= 0 }",
            r_csr_inv->prettyPrintString());
  Relation *r_coo_csr_comp = r_csr_inv->Compose(r_coo);
  EXPECT_EQ("{ [n] -> [n1] : n1 - rowcol_inv(row(n), col(n)) = 0 && n >= 0 && "
            "n1 >= 0 && n1 - rptr(row(n)) >= 0 && -n + NNZ - 1 >= 0 && -n1 + "
            "NNZ - 1 >= 0 && -n1 + rptr(row(n) + 1) - 1 >= 0 }",
            r_coo_csr_comp->prettyPrintString());

  delete r_coo;
  delete r_csr;
  delete r_csr_inv;
  delete r_coo_csr_comp;
}

TEST_F (SparseFormatTest, COO_DENSE_COMPOSE){
  Relation *dense = new Relation("{ [n1] -> [i,j] :"
		  " n1 = i * 9 + j }");

  EXPECT_EQ ("{ [n1] -> [i, j] : n1 - 9 i - j = 0 }" 
		  ,dense->prettyPrintString());
  Relation *coo_inv = new Relation("{[i,j] -> [n2] : rowcol_inv(n2,0) = i && rowcol_inv(n2,1) = j "
                               "&& 0 <= n2 && n2 < NNZ }");
  Relation *result = coo_inv->Compose(dense);

  EXPECT_EQ ("{ [n1] -> [n2] : n1 - 9 rowcol_inv(n2, ) - rowcol_inv(n2, 1) = 0 && n2 >= 0 && -n2 + NNZ - 1 >= 0 }"
		  ,result->prettyPrintString());

}


// INCOMPLETE: Test of COO to BCSR
TEST_F(SparseFormatTest, COO_BCSR) {
  // note 9 is a replacement for row: R
  // 7 for column: C
  Relation *Flatten = new Relation("{ [n] -> [ti,tj,iz,jz,in,zn,riz,rjz,rin,rjn] :"
		  " n = iz * 9 + jz }");

  EXPECT_EQ ("{ [n] -> [ti, tj, iz, jz, in, zn,"
		  " riz, rjz, rin, rjn] : n - 9 iz - jz = 0 }" 
		  ,Flatten->prettyPrintString());


  Relation *Zeros = new Relation(
		  " { [i,j] -> [ti, tj, iz, jz, in, zn, riz, rjz, rin, rjn]: "
		  " Dense (i,j) = 0 && Dense(in,jn) != 0  && iz = i && jz = j"
		  " && i = ti * 9 + riz && j = ti * 7 + rjz && in = ti * 9 + rin"
		  " && in = ti * 7 + rin && 0 <= riz && riz <= 9 && 0 <= rjz "
		  " && rjz <= 7 && 0 <= rin && rin <= 9 && 0 <= rjz && rjz<=7"
		  "}"
		  );

  EXPECT_EQ (   "{ [i, j] -> [ti, tj, iz, jz, in, zn, riz, rjz, rin, rjn]"
		" : Dense(i, j) = 0 && Dense(in, jn) = 0 && i - iz = 0 &&"
		" j - jz = 0 && i - 9 ti - riz = 0 && j - 7 ti - rjz = 0"
		" && 7 ti - in + rin = 0 && 9 ti - in + rin = 0 && riz >= 0"
		" && rjz >= 0 && rin >= 0 && -riz + 9 >= 0 && -rjz + 7 >= 0"
		" && -rin + 9 >= 0 }"
  		  ,Zeros->prettyPrintString());
}
