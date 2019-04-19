// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// write_png_core
void write_png_core(NumericVector vec, IntegerVector dims, std::string filename, bool convert_to_row_major, bool flipy, double intensity_factor, Rcpp::Nullable<Rcpp::IntegerMatrix> pal);
RcppExport SEXP _foist_write_png_core(SEXP vecSEXP, SEXP dimsSEXP, SEXP filenameSEXP, SEXP convert_to_row_majorSEXP, SEXP flipySEXP, SEXP intensity_factorSEXP, SEXP palSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< NumericVector >::type vec(vecSEXP);
    Rcpp::traits::input_parameter< IntegerVector >::type dims(dimsSEXP);
    Rcpp::traits::input_parameter< std::string >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< bool >::type convert_to_row_major(convert_to_row_majorSEXP);
    Rcpp::traits::input_parameter< bool >::type flipy(flipySEXP);
    Rcpp::traits::input_parameter< double >::type intensity_factor(intensity_factorSEXP);
    Rcpp::traits::input_parameter< Rcpp::Nullable<Rcpp::IntegerMatrix> >::type pal(palSEXP);
    write_png_core(vec, dims, filename, convert_to_row_major, flipy, intensity_factor, pal);
    return R_NilValue;
END_RCPP
}
// write_pnm_core
void write_pnm_core(NumericVector vec, IntegerVector dims, std::string filename, bool convert_to_row_major, bool flipy, double intensity_factor, Rcpp::Nullable<Rcpp::IntegerMatrix> pal);
RcppExport SEXP _foist_write_pnm_core(SEXP vecSEXP, SEXP dimsSEXP, SEXP filenameSEXP, SEXP convert_to_row_majorSEXP, SEXP flipySEXP, SEXP intensity_factorSEXP, SEXP palSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< NumericVector >::type vec(vecSEXP);
    Rcpp::traits::input_parameter< IntegerVector >::type dims(dimsSEXP);
    Rcpp::traits::input_parameter< std::string >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< bool >::type convert_to_row_major(convert_to_row_majorSEXP);
    Rcpp::traits::input_parameter< bool >::type flipy(flipySEXP);
    Rcpp::traits::input_parameter< double >::type intensity_factor(intensity_factorSEXP);
    Rcpp::traits::input_parameter< Rcpp::Nullable<Rcpp::IntegerMatrix> >::type pal(palSEXP);
    write_pnm_core(vec, dims, filename, convert_to_row_major, flipy, intensity_factor, pal);
    return R_NilValue;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_foist_write_png_core", (DL_FUNC) &_foist_write_png_core, 7},
    {"_foist_write_pnm_core", (DL_FUNC) &_foist_write_pnm_core, 7},
    {NULL, NULL, 0}
};

RcppExport void R_init_foist(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
