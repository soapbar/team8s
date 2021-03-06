% Initialize current location maze array
% 1 means unvisited
% 0.5 means visited
% 0 is the robot's current location

curr_loc = [...
    1 1 1 1 
    1 1 1 1
    1 1 1 1
    1 1 1 1 
    1 1 1 0];  

y_dim = 5;
x_dim = 4;

% Set maze walls
% For each grid space, wall locations are represented with 4 bits:
% [bit3 bit2 bit1 bit0] ==> [W E S N]
% N (north wall) = 0001 in binary or 1 in decimal
% S (south wall) = 0010 in binary or 2 in decimal
% E (east wall)  = 0100 in binary or 4 in decimal
% W (west wall)  = 1000 in binary or 8 in decimal
% For example: A grid that has a wall to the north and east would have a
% wall location of 0101 in binary or 5 in decimal

% Two wall configurations, for testing
% You can uncomment any of the wall_loc arrays or write your own mazes

wall_loc = [...
    9 1 3 5
    8 6 13 12
    8 3 6 12
    8 3 7 14
    10 3 3 7];

% User-defined color map
% 0   -> black
% 0.5 -> green
% 1   ->
map = [...
       0, 0, 0 
       0, 1, 0
       1, 1, 1];

% Draw initial grid
colormap(map);
imagesc(curr_loc);
caxis([0 1])
draw_walls(wall_loc);

% Total steps taken
steps = 0;
% Current position
curr_x = 4;
curr_y = 5;

next_x = curr_x;
next_y = curr_y;

%Direction facing: N, S, E, W
curr_dir = 'N';

segments = [];
settled = [];
frontier = [];
walls = 0.5*ones(y_dim,x_dim);

while (true)
    
    % Update node ID to current position
    ID= getNodeID(curr_x,curr_y);
    
    walls(curr_y,curr_x) = wall_loc(curr_y,curr_x);
    
    % Get wall information at current location
    wall_bin = de2bi(wall_loc(curr_y,curr_x), 4, 'right-msb');
    % Add segments connecting to all adjacent nodes
    if (wall_bin(1) == 0) %North neighbor
        segments= addSegment(segments,ID,getNodeID(curr_x,curr_y-1));
    end
    if (wall_bin(2) == 0) %South neighbor
        segments= addSegment(segments,ID,getNodeID(curr_x,curr_y+1));
    end
    if (wall_bin(3) == 0) %East neighbor
        segments= addSegment(segments,ID,getNodeID(curr_x+1,curr_y));
    end
    if (wall_bin(4) == 0) %West neighbor
        segments= addSegment(segments,ID,getNodeID(curr_x-1,curr_y));
    end
    
%======================
%    Algorithm to determine next movement here.
%    
%    Must set next_x and next_y to the next desired locations.
%
%    Must break if the maze is fully explored.
%======================
    
    [next_x,next_y,settled,frontier] = djikstra(curr_x,curr_y,wall_bin,settled,frontier,walls);
    
    if(next_x == curr_x && next_y == curr_y)
        break
    end
    
    % Update our position
    prev_y = curr_y;
    prev_x = curr_x;
    curr_y = next_y;
    curr_x = next_x;
    
    % Update our direction based on how we've changed our position
    if (curr_x > prev_x)
        curr_dir= 'E';
    elseif (curr_x < prev_x)
        curr_dir= 'W';
    elseif (curr_y < prev_y)
        curr_dir= 'N';
    else (curr_y > prev_y)
        curr_dir= 'S';
    end
    
    % We've made a movement, add a step
    steps = steps + 1;
    
    % Update maze array
    curr_loc(curr_y, curr_x) = 0;
    curr_loc(prev_y, prev_x) = 0.5;
    
    pause(0.2);
    
    %Update drawing of the maze

    imagesc(curr_loc);
    caxis([0 1]); % Re-scale colormap to original scale
    draw_walls(wall_loc);
end
display(horzcat('The maze was successfully explored in ',num2str(steps),' steps.'));