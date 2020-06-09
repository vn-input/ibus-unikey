import unittest, os, signal, sys, time

from dogtail import tree
from dogtail.config import config
from dogtail.procedural import focus, click, run, FocusWidget, FocusDialog, FocusWindow, FocusApplication, FocusError
from dogtail.rawinput import typeText, keyCombo
from dogtail.utils import screenshot
from dogtail.predicate import GenericPredicate

# @NOTE: i'm considering move these function to be as gedit helpers so we
# can reuse them to many more test-cases

method = 'telex'

def run_ui_typing_helper(cache, text):
    if os.path.isfile(cache):
        os.remove(cache)

    pid = run('gedit {}'.format(cache))
    gedit = tree.root.application('gedit')

    focus.application('gedit')    
    focus.text()
    typeText(text)
    
    keyCombo('<Control>s')

    try:
        # @NOTE: maybe we can't access the menu and click the 
        # item `Quit` at menu `File`
    
        click.menu('File')
        click.menuItem('Quit')
    except Exception as error:
        keyCombo('<Control>q')

    try:
        with open(cache) as fd:
            return fd.read()
    except Exception as error:
        os.kill(pid, signal.SIGKILL)
        return None

class TestKeyboardTyping(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestKeyboardTyping, self).__init__(*args, **kwargs)

    def test_typing_vietnamese_style(self):
        if method == 'telex':
            steps = [
                ('xin chaof, tooi laf mootj con autobot dduowcj xaay duwngj ddeer tesst ibus',
                 'xin chào, tôi là một con autobot được xây dựng để test ibus\n')
            ]
        elif method == 'vni':
            steps = [
                ('xin chao2, to6i la2 mo65t con autobot d9uo75c xa6y du7ng5 d9e63 test ibus',
                 'xin chào, tôi là một con autobot được xây dựng để test ibus\n')
            ]

        for _ in range(loop):
            for typing, expect in steps:
                self.assertEqual(run_ui_typing_helper('/tmp/0001.txt', typing),
                                 expect)

if __name__ == '__main__':
    #import dogtail.i18n
    #dogtail.i18n.loadTranslationsFromPackageMoFiles('gedit')

    #config.debugSleep = True
    #config.debugSearching = True
    #config.debugTranslation = True
    #config.actionDelay = 0.1

    if len(sys.argv) > 2:
        loop = int(sys.argv.pop())
    else:
        loop = 1

    if len(sys.argv) > 1:
        method = sys.argv.pop()
    else:
        method = 'telex'
    unittest.main()
