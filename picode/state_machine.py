import time
from Parsing.Node import Node
from Parsing.NodeMap import NodeMap
    
class FiniteStateMachine:
    def __init__(self, robot, map):
        self.robot = robot
        self.map = map
        self.current_state = None
        self.command_queue = []
        
    def fsm_loop(self):
        while (self.robot.active):
            if (not self.robot.paused):
                actions, next_state = self.get_next_state_and_actions()
                if next_state == None:
                    if not self.robot.stopped:
                        self.robot.stop()
                    continue
                if next_state == self.current_state:
                    continue
                print("Current state is {0}. Next state is {1}".format(self.current_state, next_state))
                for action in actions:
                    print("Next action is {}".format(action[0]))
                    if not self.robot.action_is_safe(action[0]):
                        print("Action is not safe. Begin waiting...")
                        self.robot.stop()
                    while not self.robot.action_is_safe(action[0]):
                        time.sleep(0.05)
                    print("Make action {0} at velocity {1}".format(action[0], action[1]))
                    self.make_action(action)    # blocks until action is complete
                self.current_state = next_state
                #robot.update_state()        # uses current state to update x,y,theta of robot
            else:
                time.sleep(0.5)
        
    def get_next_state_and_actions(self):
        next_state = [None, None]
        if len(self.command_queue) > 0:
            next_state = self.command_queue[0]
            self.command_queue = self.command_queue[1:]
        return next_state
       
    def enqueue_directions(self, start_state, end_state):
        if start_state == None:
            start_state = self.current_state
        if self.current_state == None:
            self.current_state = start_state
        directions = self.map.getStatesQueue(start_state, end_state)
        if directions == None or len(directions) == 0:
            print("Enqueuing new directions failed")
            return
        self.command_queue = self.command_queue + directions
    
    def make_action(self, action):
        if action[0] == "LaneFollowToStop":
            self.robot.lane_follow(action[1], "intersection")
        elif action[0] == "LaneFollowToTurn":
            self.robot.lane_follow(action[1], "turn")
        elif action[0] == "LaneFollowToStraight":
            self.robot.lane_follow(action[1], "straight")
        elif "Left" in action[0]:
            self.robot.make_left_turn(action[1])
        elif "Right" in action[0]:
            self.robot.make_right_turn(action[1])
        elif "Straight" in action[0]:
            self.robot.drive_straight(action[1])