"""
test for matplotlib and numpy
produce a bar plot with errorbars
@module: matplotlib_test.py
@version: 0.2 20110127
@author: Steffen Kube
@swreq SREQ:0001146: g > appserver test
"""

class test:
	def __init__(self, filename, outputfile=None, options=None):
		self.infile=filename
		self.inoptions=options

	def start(self):
		print "test NUMPY"
		import numpy as np
		
		N = 5
		menMeans = (20, 35, 30, 35, 27)
		menStd =   (2, 3, 4, 1, 2)
		ind    = np.arange(N)  # NUMPY: the x locations for the groups
		print "  numpy o.k."
		
		print "test MATPLOTLIB"
		
		import matplotlib.pyplot as plt
		
		width = 0.35       # the width of the bars
		fig   = plt.figure()
		ax    = fig.add_subplot(111)
		rects1= ax.bar(ind, menMeans, width, color='r', yerr=menStd)
		
		# comment this out, if you want to see the graphics
		# plt.show()
		
		print " matplotlib.pyplot  o.k."
		
		from matplotlib.figure import Figure
		fig = Figure()
		
		result = {'answer':'o.k.'}
		
		return result # dictionary


