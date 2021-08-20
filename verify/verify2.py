
import os
import sys
import subprocess
import string

traces_dir = './traces'
executable_path = './build/branchsim'

configs = {
           1: ['-O 1 -G 12 -D', '-D -O 1 -G 15', '-D -O 1 -G 8', '-N 10'],
           2: ['-D -O 2 -G 12', '-D -O 2 -G 15 -P 10', '-D -O 2 -G 8 -P 12', '-D -O 2 -N 10'],
           3: ['-D -O 3 -G 59', '-D -O 3 -G 59 -P 8', '-D -O 3 -G 24 -P 10', '-D -O 3 -G 59 -N 10'],
          }

configname = {1: 'gshare', 2: 'yehpatt', 3: 'perceptron'}

def usage():
    print("""Usage: python3 verify2.py <config>
Specify which branch pedictor you want to run:
    1: GShare
    2: Yeh-Patt
    3: Perceptron""")

if len(sys.argv) != 2 or sys.argv[1] == '-h':
    usage()
    sys.exit(1)

try:
    user_conf = int(sys.argv[1])
except ValueError:
    print("Unable to parse argument: {}".format(sys.argv[1]))
    usage()
    sys.exit(1)

if user_conf < 1 or user_conf > 3:
    print("Invalid config: {}".format(user_conf))
    usage()
    sys.exit(1)

verification_out_filename = '{}.out'.format(configname[user_conf])
print('Writing to: {}'.format(verification_out_filename))

# Get traces and configuration files
trace_files = []
for dirpath,_, files in os.walk(traces_dir):
    for f in files:
        fname = (os.path.abspath(os.path.join(dirpath, f)))
        if fname.endswith('.trace'):
            trace_files.append(fname)

trace_files.sort()

# Check if executable exists
if not os.path.exists(executable_path):
    sys.exit('The executable branchsim must exists at location ' + executable_path)

# Remove the verification file if it exists
if os.path.exists(verification_out_filename):
    os.remove(verification_out_filename)

f = open(verification_out_filename, 'a+')

for conf in configs[user_conf]:
    for trace in trace_files:
        arg_list = conf.split()
        arg_list.insert(0, executable_path)
        arg_list.append('-i')
        arg_list.append(trace)
        print('Running: ', arg_list)
        cp = subprocess.run(arg_list, universal_newlines=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # print(run_args)
        # Make sure there were no errors
        if len(cp.stderr) == 0:
            f.write('TRACE: ' + os.path.basename(trace) + '\n')
            f.write('CONF: ' + os.path.basename(conf) + '\n')
            f.write(cp.stdout)
            f.write('\n\n\n')
        else:
            print(cp.stderr)

