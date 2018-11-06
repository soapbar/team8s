function [next_x,next_y] = djikstra(curr_x,curr_y,wall_bin,settled,frontier)
    
    if(wall_bin(1) == 0)
        frontier(end+1,2) = curr_y - 1;
        frontier(end+1,1) = curr_x;
    elseif(wall_bin(4) == 0)
        next_x = curr_x - 1;
        next_y = curr_y;
        frontier(end+1,2) = curr_y - 1;
        frontier(end+1,1) = curr_x;
        if(next_x == 0)
            break
        end
    elseif(wall_bin(2) == 0)
        next_y = curr_y + 1;
        next_x = curr_x;
        if(next_y == 6)
            break
        end
    elseif(wall_bin(3) == 0)
        next_x = curr_x + 1;
        next_y = curr_y;
        if(next_x >= x_dim+1)
            break
        end
    end
    
    next_x = curr_x;
    next_y = curr_y;