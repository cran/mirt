% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/mirt-package.R
\docType{data}
\name{Science}
\alias{Science}
\title{Description of Science data}
\description{
A 4-item data set borrowed from \code{ltm} package in R, first example
of the \code{grm()} function. See more complete documentation therein.
}
\examples{

\dontrun{
itemstats(Science)

mod <- mirt(Science, 1)
plot(mod, type = 'trace')
}
}
\references{
Chalmers, R., P. (2012). mirt: A Multidimensional Item Response Theory
Package for the R Environment. \emph{Journal of Statistical Software, 48}(6), 1-29.
\doi{10.18637/jss.v048.i06}
}
\author{
Phil Chalmers \email{rphilip.chalmers@gmail.com}
}
\keyword{data}
