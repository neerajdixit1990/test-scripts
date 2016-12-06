import subprocess
import sys
import datetime
import os
import time

if len(sys.argv) < 3:
	print 'Correct usage python auto_bench.py <count> <filename>'
	exit(1)

#cmd = [sys.argv[1]]
#cmd = ["docker", "run", "-t", "--name=streaming_client", "-v", "/path/to/output:/output", "--volumes-from", "streaming_dataset", "--net", "streaming_network", "cloudsuite/media-streaming:client", "streaming_server"]
cmd = ["docker", "run", "--rm", "--volumes-from", "data", "cloudsuite/graph-analytics", "--driver-memory", "32g", "--executor-memory", "32g"]
n = int(sys.argv[1])
f = open(sys.argv[2], 'w')
#print cmd
#print n
#print sys.argv[3]
for i in range(n):
	os.system("docker rm -v $(docker ps -a -q -f status=exited)")

	try:
        	tic = datetime.datetime.now()
        	op = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        	toc = datetime.datetime.now()
	except subprocess.CalledProcessError as e:
    		f.write('Calledprocerr : %s' %(str(e)))

	'''
        output_list = op.split('\n')
        l = len(output_list)
        if l <= 20:
                start = 0
        else:
                start = l - 20
        for j in range(start, l):
                f.write(output_list[j])

        '''
	'''
	tic = datetime.datetime.now()
	proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	proc.wait()
	toc = datetime.datetime.now()
	temp_stdout = []
	for line in proc.stdout.readlines():
    		temp_stdout.append(line)
	l = len(temp_stdout)
	for j in range(l-20, l):
		f.write(temp_stdout[j])

	f.write('-------stderr------')
	
	temp_stderr = []
	for line in proc.stderr.readlines():
		temp_stderr.append(line)	
	l = len(temp_stderr)
	if l <= 20:
		start = 0
	else:
		start = l - 20
	for j in range(start, l):
		f.write(temp_stderr[j])
	'''
        f.write(op)
	f.write('\nExecution time %s\n' %(toc-tic))
	f.write('\n--------------------------------\n')
	time.sleep(5) 
f.close()
