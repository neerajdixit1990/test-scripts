import psutil

def log_statistics():
	# CPU statistics
	print 'No of logical CPUs is %s' %psutil.cpu_count(logical=True)
	print 'CPU times as reported is %s' %str(psutil.cpu_times())
	print 'CPU usage is %s' %psutil.cpu_percent(percpu=True)
	print 'CPU stats are as follows: %s' %str(psutil.cpu_stats())
	print ''
	print ''

	# Memory statistics
	print 'Virtual memory at this point %s' %str(psutil.virtual_memory())
	print 'Swap memory at this point %s' %str(psutil.swap_memory())
	print ''
	print ''

	# Disk statistics
	print 'Disk partition is ' 
	for entry in psutil.disk_partitions():
		print entry
		print 'Usage of %s is %s' %(entry[1], str(psutil.disk_usage(entry[1])))
	print ''
	print ''

	#print 'Disk usage is %s' %str(psutil.disk_usage('/'))

log_statistics()
