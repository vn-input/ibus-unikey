import unittest, os, signal

from dogtail import tree
from dogtail.config import config
from dogtail.procedural import focus, click, run
from dogtail.rawinput import typeText
from dogtail.utils import screenshot
from dogtail.predicate import GenericPredicate

# @NOTE: i'm considering move these function to be as gedit helpers so we
# can reuse them to many more test-cases

def run_ui_typing_helper(cache, text):
    if os.path.isfile(cache):
        os.remove(cache)

    pid = run('gedit')
    gedit = tree.root.application('gedit')
    saved = False
    ready_to_save = False

    try:
        focus.application('gedit')

        focus.text()
        typeText(text)

        # Click gedit's Save button.
        click.button('Save')

        try:
            # Focus gedit's Save As... dialog

            focus.widget.findByPredicate(GenericPredicate(roleName='file chooser'))
        finally:
            ready_to_save = True
    except FocusError as error:
        try:
            # This string changed somewhere around gedit 2.13.2.
            # This is the new string

            focus.dialog('Save As\u2026')
        except FocusError:
            # Fall back to the old string.

            focus.dialog('Save as...')
        finally:
            ready_to_save = True
    finally:
        try:
            if ready_to_save:
                try:
                    typeText(cache)
                    click('Save')
                finally:
                    saved = True

            # Let's quit now.
            try:
                click('File')
            except Exception as error:
                print(error)
            finally:
                click('Quit')
        except Exception as error:
            print(error)
            os.kill(pid, signal.SIGKILL)

    if saved:
        with open(cache) as fd:
            return fd.read()
    else:
        return None

class TestKeyboardTyping(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestKeyboardTyping, self).__init__(*args, **kwargs)

    def test_typing_to_gedit(self):
        steps = [
            ('i\'m typing \'hello world\' using dogtail',
             'i\'m typing \'hello world\' using dogtail\n')
        ]

        for typing, expect in steps:
            result = run_ui_typing_helper('/tmp/test_typing_to_gedit.txt',
                                          typing)
            self.assertEqual(result, expect)

if __name__ == '__main__':
    #import dogtail.i18n
    #dogtail.i18n.loadTranslationsFromPackageMoFiles('gedit')

    #config.debugSleep = True
    #config.debugSearching = True
    #config.debugTranslation = True

    unittest.main()
