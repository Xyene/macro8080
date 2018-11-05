import urllib2
from bs4 import BeautifulSoup as BS
import re

html = urllib2.urlopen("http://pastraiser.com/cpu/i8080/i8080_opcodes.html").read().replace('<br>', '\n')

soup = BS(html, 'lxml')

ops = []

wide_trans = {
    'PSW': 'AF',
    'B': 'BC',
    'D': 'DE',
    'H': 'HL',
}

table = soup.find('table')
for row in table.findAll('tr')[1:]:
    for cell in row.findAll('td')[1:]:
        is_wide = cell['bgcolor'] in ('#ccffcc', '#ffcccc')
        op_data = cell.text.split('\n')[0]
        op_data = op_data.lstrip('*')
        op_data = map(str, re.split('[,\s]', op_data))

        for i, w in enumerate(op_data):
            if is_wide:
                op_data[i] = wide_trans.get(w, op_data[i])

        ops.append(op_data)

print len(ops)

func_defs = []
func_labels = []
lookup_table = []

def param_map(x):
    if x in ('a16', 'd16', 'd8'):
        return '_' + x
    if x in '7531':
        return 'num'
    return x

for op in ops:
    name = op[0]
    data = map(param_map, op[1:])

    trans = [['X', 'Y'], ['WX', 'WY']]

    func_labels.append('''%s: %s(%s)''' % ('_'.join(op), op[0], ', '.join(op[1:])))
    lookup_table.append('&&%s' % '_'.join(op))

    func_defs.append('''#define %s(%s) \\
    assert(false); // TODO \\
DONE
''' % (name, ', '.join(map(lambda (i, d): trans[len(d) == 2][i], enumerate(data)))))

with open('opcode_defs.h', 'w') as o:
    done = set()
    for f in func_defs:
        if f not in done:
            done.add(f)
            print>>o, f

with open('opcode_labels.h', 'w') as o:
    for l in func_labels:
        print>>o, l

with open('opcode_lookup.h', 'w') as o:
    print>>o, '''static void *ops[] = {
    %s
};''' % ', '.join(lookup_table)
