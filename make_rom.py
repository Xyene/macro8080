import argparse

def main():
    parser = argparse.ArgumentParser(description='''
        Generates a prebuilt header file representing a particular memory setup.
    ''')

    parser.add_argument('rom', type=str, nargs='+', help='<address>=<path> format to place <path> at <address> in memory.')
    parser.add_argument('-o', '--out', type=str, required=True, help='Output header file.')

    args = parser.parse_args()

    positions = []
    for rom in args.rom:
        data = rom.split('=')
        if len(data) != 2:
            parser.error('ROM should be specified in <address>=<path> format.')

        addr, path = data
        addr = int(addr, 16)
        positions.append((addr, path))

    positions.sort()

    memory_array = []
    for pos, rom in positions:
        memory_array.append('  // %s' % rom)
        with open(rom, 'r') as f:
            for i, data in enumerate(f.read()):
                memory_array.append('  [0x%04X] = 0x%02X,' % (pos + i, ord(data)))
        memory_array.append('')

    with open(args.out, 'w') as out:
        print >> out, '''#define MEM_SIZE 65536

unsigned char memory[MEM_SIZE] = {
%s
};
''' % '\n'.join(memory_array)

if __name__ == '__main__':
    main()
