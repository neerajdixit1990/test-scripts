import psutil
import logging
import time

logging.basicConfig(level=logging.DEBUG,
		format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
		#datefmt='%m-%d %H:%M',
		filename='/var/log/benchmark.log',
		filemode='w')
bench = logging.getLogger('all')

def log_statistics():
	# CPU statistics
	'''
	print 'No of logical CPUs is %s' %psutil.cpu_count(logical=True)
	print 'CPU times as reported is %s' %str(psutil.cpu_times())
	print 'CPU usage is %s' %psutil.cpu_percent(percpu=True)
	print 'CPU stats are as follows: %s' %str(psutil.cpu_stats())
	print ''
	print ''
	'''
	#hdlr = logging.FileHandler('/var/log/cpu.log')
	#cpu.addHandler(hdlr)
	bench.info('CPU usage = %s', psutil.cpu_percent(percpu=True))
	bench.info('CPU stats = %s', str(psutil.cpu_stats()))

	
	# Memory statistics
	'''
	print 'Virtual memory at this point %s' %str(psutil.virtual_memory())
	print 'Swap memory at this point %s' %str(psutil.swap_memory())
	print ''
	print ''
	'''
	#mem = getlogfile('/var/log/mem.log')
	bench.info('Virtual memory = %s', str(psutil.virtual_memory()))
	bench.info('Swap memory = %s', str(psutil.swap_memory()))
	
	
	# Disk statistics
	'''
	print 'Disk partition is ' 
	for entry in psutil.disk_partitions():
		print entry
		print 'Usage of %s is %s' %(entry[1], str(psutil.disk_usage(entry[1])))
	print ''
	print ''
	'''

while True:
	log_statistics()
	time.sleep(1)
