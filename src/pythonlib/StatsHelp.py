"""
@module: StatsHelp.py
@summary: library for statistics
@version: 2010-8-26
@author: Stefan Horatschek
"""


import numpy as np
import scipy.stats

#Berechnet arithmetisches Mittel einer Liste
def List_mean(x):
	n=len(x)
	return(float(sum(x))/n)

#Berechnet korrigierte Stichprobenvarianz (Methode n-1) einer Liste
def List_var(x):
	x_quer=List_mean(x)
	n=len(x)
	quad_abweichung=[(x_i-x_quer)**2 for x_i in x]
	return(sum(quad_abweichung)/(n-1))

#Berechnet die Wurzel der korrigierte Stichprobenvarianz (Methode n-1), d.h. Standardabweichung, einer Liste
def List_sd(x):
	return(List_var(x)**0.5)

#Berechnet die Spannweite der Werte einer Liste (range)
def List_max_minus_min(x):
	return(max(x)-min(x))

#berechnet (korrigierte) Varianz eines numpy-arrays
def var(x,cor=True):
	v=np.var(x)
	if cor==True:
		n=len(x)
		return (v*n)/(n-1)
	else:
		return v

#berechnet (korrigierte) Standardabweichung eines numpy-arrays
def sd(x,cor=True):
	return var(x,cor)**0.5

#berechnet (korrigierten) Variationskoeffizienten (CV) eines numpy-arrays
def CV(x,cor=True):
	return sd(x,cor)/np.mean(x)

#berechnet (korrigierte) Schiefe eines numpy-arrays [siehe auch http://en.wikipedia.org/wiki/Skewness]
def skewness(x,cor=True):
	n=len(x)
	skew=np.sum((x-np.mean(x))**3)/n/(np.var(x)**1.5)
	if cor==True:
		return skew*(n*(n-1))**0.5/(n-2)
	else:
		return skew

#berechnet das Konfidenz-Intervall fuer den Mittelwert einer normalverteilten Zufallsgroesse zum Konfidenzniveau beta (bei unbekannter Varianz)
#getestet mit Wikipedia-Beispiel
def KI_mean(x,beta):
	s=sd(x)
	xquer=np.mean(x)
	n=len(x)
	c=scipy.stats.t.ppf(0.5*(1+beta),n-1)
	return (xquer-c*s/np.sqrt(n), xquer+c*s/np.sqrt(n))

#berechnet Konfidenz-Intervall-Laenge
def KI_mean_length(x,beta):
	KI=KI_mean(x,beta)
	return KI[1]-KI[0]

#berechnet das Konfidenz-Intervall fuer die Varianz einer normalverteilten Zufallsgroesse zum Konfidenzniveau beta
def KI_var(x,beta):
	s2=var(x,True)
	n=len(x)
	a1=s2*(n-1)/scipy.stats.chi2.ppf(0.5*(1+beta),n-1)
	a2=s2*(n-1)/scipy.stats.chi2.ppf(0.5*(1-beta),n-1)
	return (a1,a2)

#berechnet das Konfidenz-Intervall fuer die Standardabweichung einer normalverteilten Zufallsgroesse zum Konfidenzniveau beta
def KI_sd(x,beta):
	KI_Varianz=KI_var(x,beta)
	(a1, a2)=KI_Varianz
	return (a1**0.5,a2**0.5)

#berechnet den Anteil der x-Werte kleiner als x0
def fraction_under(x,x0):
	return float(len(x[x<x0]))/len(x)

#berechnet den Anteil der x-Werte groesser als x0
def fraction_over(x,x0):
	return float(len(x[x>x0]))/len(x)

#schaetzt den Anteil der x-Werte kleiner als x0 unter der Annahme der Normalverteilung
def fraction_under_normal(x,x0):
	mu=np.mean(x)
	sigma=sd(x)
	return scipy.stats.norm.cdf((x0-mu)/sigma)

#schaetzt den Anteil der x-Werte kleiner als x0 unter der Annahme der Normalverteilung
def fraction_over_normal(x,x0):
	mu=np.mean(x)
	sigma=sd(x)
	return scipy.stats.norm.sf((x0-mu)/sigma)

# Modul
#' Funktion berechnet, ob pima-Messwert vgl mit BD-Messwert im ATE liegt, liefert TRUE/FALSE
#' Hinweis: Jens meint fuer bd>=1000 gelten nun auch +/-30%
#'
#' @param pima numeric value of pima cd3cd4-count
#' @param bd numeric value of bd cd3cd4-count
def inATE(pima,bd):
	ATEnumber = {
	    'bd_val1':200, #bereich 1: 0<=bd<bd_val1
	    'bd_val2':1000,#bereich 2: bd_val1<=bd<bd_val2, bereich 3: sonst
	    'bereich1_max_diff': 60, #zulaessige Differenz in Bereich 1
	    'bereich2_max_prozent' : 30, #zulaessige Abweichung in Bereich 2
	    'bereich3_max_prozent' : 40  #zulaessige Abweichung in Bereich 3
	}
	if ((bd<ATEnumber['bd_val1']) and (abs(bd-pima)<=ATEnumber['bereich1_max_diff'])):
		return(True)
	if ((ATEnumber['bd_val1']<=bd<ATEnumber['bd_val2']) and (abs(1-pima/bd)<=0.01*ATEnumber['bereich2_max_prozent'])):
		 return(True)
	if ((bd>=ATEnumber['bd_val2']) and (abs(1-pima/bd)<=0.01*ATEnumber['bereich3_max_prozent'])):
		 return(True)
	return(False)

def is_equal(float1,float2,prec=1e-5):
	if abs(1-float1/float2)<prec:
		return(True)
	else:
		return(False)

def QCtext(value):
	if value==1:
		return("passed")
	if value==0:
		return("not passed")
	if is_equal(value,0.5):
		return("Sonderfreigabe")
	return("Bedeutung von QCvalue="+str(value)+" unbekannt.")

#liefert kleinsten kritischen Wert c (Ablehnbereich c...n), fuer Binomialtest H0:p<=p0, H1:p>p0, Stichprobenumfang n, Signifikanzniveau alpha>0
def binomialtest_H0_p_kleinergleich_p0_ablehnbereich(p0,n,alpha):
	#kleinster Wert c, bei dem sum_i=c ^n stats.binom.pmf(i,n,p0)<=alpha
	c=n
	#Schleife leifert (ersten) groessten Wert, der Bdg nicht erfuellt
	while scipy.stats.binom.sf(c-1,n,p0)<=alpha: #summe von c bis n (summe von n bis n enthaelt einen beitrag)
#		print "c, value, alpha", c, stats.binom.sf(c-1,n,p0), alpha,  c-1, stats.binom.sf(c-2,n,p0)
		c-=1
	c=c+1
	return range(c,n+1) #c..n
#theoretisch kann c=n+1 [Bsp: binomialtest_obereSchranke(0.5,1,0.1)] werden, dass heisst H0 wird bei beliebiger Stichprobeausgang angenommen

#liefert groessten kritischen Wert c (Ablehnbereich 0...c), fuer Binomialtest H0:p>=p0, H1:p<p0, Stichprobenumfang n, Signifikanzniveau alpha>0
def binomialtest_H0_p_groessergleich_p0_ablehnbereich(p0,n,alpha):
	#groesster Wert c, bei dem sum_i=0 ^c stats.binom.pmf(i,n,p0)<=alpha
	c=0
	#Schleife leifert (ersten) groessten Wert, der Bdg nicht erfuellt
	while scipy.stats.binom.cdf(c,n,p0)<=alpha: #summe von c bis n (summe von n bis n enthaelt einen beitrag)
		c+=1
	c=c-1
	return range(0,c+1) #0..c
#theoretisch kann c=-1 [Bsp: binomialtest_untereSchranke(0.5,1,0.1)] werden, dass heisst H0 wird bei beliebigen Stichprobeausgang angenommen

#berechnet exaktes konfidenzintervall fuer die Aufrtittswahrscheinlichkeit bei k auftreten von n Versuchen zum Niveaue beta
#opt=0 [pu..po], opt=1 [0..po], opt=-1 [pu..1]
def Clopper_Pearson_Intervall(k,n,beta,opt=0):
	if k==0:
		opt=1
	elif k==n:
		opt=-1
	if opt==0:
		if k==0:
			pu=0
		else:
			pu=scipy.stats.beta.ppf(0.5*(1.0-beta),k,n-k+1)
		if k==n:
			po=1
		else:
			po=scipy.stats.beta.ppf(1.0-0.5*(1.0-beta),k+1,n-k)
	elif opt==1:
		pu=0
		po=scipy.stats.beta.ppf(beta,k+1,n-k)
	elif opt==-1:
		pu=scipy.stats.beta.ppf(1.0-beta,k,n-k+1)
		po=1
	return [pu,po]
