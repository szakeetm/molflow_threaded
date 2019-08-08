import pyautogui
import subprocess
import os
import time
import filecmp
import sys, getopt

software_name = "Molflow+"
IMAGE_LOCATION = os.path.dirname(__file__)+"/Images/"
N_LIMIT = "100"
load_file = ""
save_file = ""

def checkExist(location, message):
    if location is None:
        print(message)
        exit(0)
    return 1

def find_molflow_with_image():
    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_inactive.png')
    if mf_location is not None:
        inactive_center = pyautogui.center(mf_location)
        pyautogui.click(inactive_center)

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_active.png')
    checkExist(mf_location, "Molflow not active")

def find_molflow():
    # see if molflow is inactive and activate it
    process = subprocess.run(["wmctrl", "-a", software_name])
    if process.returncode:
        print("Molflow process not found")
        exit(1)
    #maximize window
    subprocess.run(["wmctrl","-r", software_name,"-b","add,maximized_vert,maximized_horz"])

    time.sleep(0.5)
    #mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_toolbar.png')
    #checkExist(mf_location, "Molflow toolbar not found")

    #mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_status.png')
    #checkExist(mf_location, "Molflow window not found")

    #maximize window and get focus
    #pyautogui.click(pyautogui.center(mf_location))
    return 1

def load_from_file():
    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'menu_file.png')
    checkExist(mf_location, "Couldn't find file menu")
    pyautogui.click(pyautogui.center(mf_location))

    pyautogui.keyDown('ctrl')
    pyautogui.press(['o'])
    pyautogui.keyUp('ctrl')

    pyautogui.keyDown('ctrl')
    pyautogui.press(['l'])
    pyautogui.keyUp('ctrl')

    time.sleep(0.5)
    global load_file
    pyautogui.typewrite(load_file, interval=0.01)
    pyautogui.press('enter')

    #mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_testfile.png')
    #checkExist(mf_location, "File not found")
    #pyautogui.doubleClick(pyautogui.center(mf_location))

#set simulation limit to 100 desorptions
def check_subprocesses():
    # set simulation limit
    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_simulation.png')
    checkExist(mf_location, "Simulation menu button not found")
    # a simple click doesn't work here
    pyautogui.move(pyautogui.center(mf_location))
    pyautogui.click(pyautogui.center(mf_location))

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'field_n_subprocesses.png')
    checkExist(mf_location, "Couldn't verify correct amount of subprocesses")

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_limit.png')
    checkExist(mf_location, "Couldn't find button for desorption limit")
    pyautogui.click(pyautogui.center(mf_location))

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'field_limit.png')
    checkExist(mf_location, "Couldn't find for desorption limit")
    pyautogui.doubleClick(mf_location[0]+mf_location[2]+10, pyautogui.center(mf_location).y, interval=0.05)

    time.sleep(0.5)
    pyautogui.press('backspace')
    pyautogui.typewrite(N_LIMIT)
    pyautogui.press('enter')

    #mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_ok.png')
    #checkExist(mf_location, "Couldn't find ok button")
    #pyautogui.click(pyautogui.center(mf_location))

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'mf_subwindow_state.png')
    checkExist(mf_location, "Couldn't find window status menu")
    #pyautogui.move(pyautogui.center(mf_location))
    pyautogui.click(mf_location[0]+mf_location[2]-5,pyautogui.center(mf_location).y)

def run_simulation():
    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_begin.png')
    checkExist(mf_location, "Couldn't find begin button")
    pyautogui.click(pyautogui.center(mf_location))

    time.sleep(5.0)

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_ok.png')
    if mf_location is None:
        mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION + 'btn_resume.png')
        checkExist(mf_location, "Couldn't verify if simulation finished")
    else:
        pyautogui.click(pyautogui.center(mf_location))

def save_to_file():
    global save_file
    if os.path.isfile(save_file):
        os.remove(save_file) # make sure the file gets removed

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'menu_file.png')
    checkExist(mf_location, "Couldn't find file menu")
    pyautogui.click(pyautogui.center(mf_location))

    mf_location = pyautogui.locateOnScreen(IMAGE_LOCATION+'btn_saveas.png')
    checkExist(mf_location, "Couldn't find save as menu")
    pyautogui.click(pyautogui.center(mf_location))


    pyautogui.typewrite(save_file, interval=0.01)
    pyautogui.press('enter')

def start_test():

    load_from_file()

    check_subprocesses()

    run_simulation()

    save_to_file()


def get_inputfile(argv):
    inputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:", ["ifile="])
    except getopt.GetoptError:
        print('molflow_test.py -i <inputfile>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('molflow_test.py -i <inputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
    return inputfile

def main(argv):
    # for now assume that molflow is running in start position
    #test_file = "results10.100_tex.xml"
    test_file = get_inputfile(argv)

    global load_file
    global save_file
    global gold_file
    absolute_dir_path = os.path.abspath(os.path.dirname(__file__))
    load_file = absolute_dir_path + "/TestFiles/" + test_file
    save_file = absolute_dir_path + "/TestFiles/test_" + test_file
    gold_file = absolute_dir_path + "/TestFiles/gold_" + test_file

    #pyautogui.alert(os.path.dirname(__file__))
    pyautogui.PAUSE = 0.4
    if find_molflow():
        start_test()

    time.sleep(3.0)
    message = ""
    cmp_result = filecmp.cmp(save_file, gold_file)
    if cmp_result:
        message = "Generated test files are identical!"
        print(message)
        return 0
    else:
        message = "Generated test files are different!"
        print(message)
        return 1



if __name__== "__main__":
    print('Starting py main')
    returnval_of_main = main(sys.argv[1:])
    print('Exiting py main')
    exit(returnval_of_main)