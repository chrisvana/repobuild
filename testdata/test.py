import a.test
import common.base.flags as flags
import testdata.a.a_pb2
import sys

flags.d.DEFINE_string('pass_text', 'PASS', 'What to print')

def main(argv):
    flags.Parse(argv)
    testdata.a.a_pb2.FooProto()
    print(flags.FLAGS.pass_text)

if __name__ == "__main__":
    main(sys.argv)
