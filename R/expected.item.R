#' Function to calculate expected value of item
#'
#' Given an internal mirt object extracted from an estimated model
#' compute the expected value for an item given the ability parameter(s).
#'
#' @aliases expected.item
#' @param x an extracted internal mirt object containing item information (see \code{\link{extract.item}})
#' @param Theta a vector (unidimensional) or matrix (multidimensional) of latent trait values
#' @param min a constant value added to the expected values indicating the lowest theoretical
#'   category. Default is 0
#' @param include.var logical; include the model-implied variance of the expected scores as well?
#'   When \code{TRUE} will return a list containing the expected values (\code{E}) and variances
#'   (\code{VAR})
#'
#' @author Phil Chalmers \email{rphilip.chalmers@@gmail.com}
#' @references
#' Chalmers, R., P. (2012). mirt: A Multidimensional Item Response Theory
#' Package for the R Environment. \emph{Journal of Statistical Software, 48}(6), 1-29.
#' \doi{10.18637/jss.v048.i06}
#' @keywords expected value
#' @export expected.item
#' @seealso \code{\link{extract.item}}, \code{\link{expected.test}}
#' @examples
#'
#' mod <- mirt(Science, 1)
#' extr.2 <- extract.item(mod, 2)
#' Theta <- matrix(seq(-6,6, length.out=200))
#' expected <- expected.item(extr.2, Theta, min(Science[,1])) #min() of first item
#' head(data.frame(expected, Theta=Theta))
#'
#' expected.item(extr.2, Theta, min(Science[,1]), include.var=TRUE)
#'
expected.item <- function(x, Theta, min = 0, include.var = FALSE){
    if(missing(x)) missingMsg('x')
    if(missing(Theta)) missingMsg('Theta')
    if(is(Theta, 'vector')) Theta <- as.matrix(Theta)
    if(!is.matrix(Theta)) stop('Theta input must be a matrix', call.=FALSE)
    tmp <- try(x@nfact, TRUE)
    if(!is(tmp, 'try-error'))
        if(ncol(Theta) != x@nfact)
            stop('Theta does not have the correct number of dimensions', call.=FALSE)
    P <- ProbTrace(x=x, Theta=Theta)
    Emat <- matrix(0:(x@ncat-1), nrow(P), ncol(P), byrow = TRUE)
    E <- rowSums(P * Emat) + min
    if(include.var){
        V <- rowSums(P * (Emat-E)^2)
        ret <- list(E=E, VAR=V)
        return(ret)
    }
    E
}
