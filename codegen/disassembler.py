# reads the HTML file with language ref and produces codegen
import sys
from html.parser import HTMLParser

class OpcodeParser(HTMLParser):

    def __init__(self):
        super().__init__()
        self.in_table = False
        self.rows = []
        self.row = 0
        self.col = 0

    def start_table(self):
        self.in_table = True

    def end_table(self):
        self.in_table = False

    def start_row(self):
        self.col = 0;
        self.rows.append([None, None, None, None, None, None])

    def end_row(self):
        pass

    def start_col(self):
        # don't start the table until we've seen our first TD
        self.start_table()
        pass

    def end_col(self):
        self.col = self.col + 1

    def set_data(self, data):
        if self.in_table:
            self.rows[-1][self.col] = data.strip()

    def handle_starttag(self, tag, attrs):
        if tag == 'table':
            pass
        elif tag == 'tr':
            self.start_row()
        elif tag == 'td':
            self.start_col()

    def handle_endtag(self, tag):
        if tag == 'table':
            self.end_table()
        elif tag == 'tr':
            self.end_row()
        elif tag == 'td':
            self.end_col()

    def handle_data(self, data):
        self.set_data(data)

class Instruction(object):

    def __init__(self, row):
        self.opcode = row[0]
        self.instruction = row[1]
        self.size = row[2] or '1'
        self.flags = row[3]
        self.function = row[4]
        self.junk = row[5]

    def print(self):
        if self.instruction == '-':
            self.instruction = 'NOP'
        # case 0x00: printf("NOP"); break;
        #case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        if self.size == '1':
            print('case {}: printf("{}"); break;'.format(self.opcode, self.instruction))
        elif self.size == '2':
            # 'LXI B,D16' should become 'LXI B,#$%02x%02x'
            parts = self.instruction.split(',')
            if len(parts) > 1:
                self.instruction = parts[0]
                args = ' '.join(parts[1:])
            print('case {}: printf("{}, #$%02x", code[1]); opbytes = 2; break;'.format(self.opcode, self.instruction))
        elif self.size == '3':
            # 'LXI B,D16' should become 'LXI B,#$%02x%02x'
            parts = self.instruction.split(',')
            if len(parts) > 1:
                self.instruction = parts[0]
                args = ' '.join(parts[1:])
            print('case {}: printf("{}, #$%02x%02x", code[2], code[1]); opbytes = 3; break;'.format(self.opcode, self.instruction))
        else:
            print(self.opcode, self.instruction, self.size, '??')


parser = OpcodeParser()

with open(sys.argv[1]) as h:
    d = h.read()
parser.feed(d)

for row in parser.rows:
    i = Instruction(row)
    i.print()
