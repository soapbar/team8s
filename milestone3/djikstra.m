function [next_x,next_y,settled,frontier] = djikstra(curr_x,curr_y,wall_bin,settled,frontier,walls)
    
    settled(end+1,1:2) = [curr_x,curr_y];

    % Add open spaces to frontier set
    if(wall_bin(1) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = curr_x;
        frontier(nr_f+1,2) = curr_y - 1;
    end
    if(wall_bin(2) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = curr_x;
        frontier(nr_f+1,2) = curr_y+1;
    end
    if(wall_bin(3) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = curr_x+1;
        frontier(nr_f+1,2) = curr_y;
    end
    if(wall_bin(4) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = curr_x-1;
        frontier(nr_f+1,2) = curr_y;
    end
    
    % Remove already settled nodes
    [nr_s,~] = size(settled);
    for i = 1:nr_s
        point = settled(i,:);
        [nr_f,~] = size(frontier);
        c = 1;
        while c <= nr_f
            if isequal(point,frontier(c,1:2))
                if c == 1 
                    frontier = frontier(2:end,:);
                elseif c == nr_f
                    frontier = frontier(1:end-1,:);
                else
                    frontier = [frontier(1:c-1,:);frontier(c+1:end,:)];
                end
            else
                c = c+1;
            end
            [nr_f,~] = size(frontier);
        end
    end
    
    [numLeft,~] = size(frontier);
    if numLeft == 0
        next_x = curr_x;
        next_y = curr_y;
    else
        dists = abs(frontier(:,1)-curr_x)+abs(frontier(:,2)-curr_y);
        [~,ind] = min(dists);
        dest = [frontier(ind,1),frontier(ind,2)]; % Choose next frontier to visit
        start = [curr_x,curr_y];
        path = [start,0];
        path = findPath(dest,start,walls,path);
        next_x = path(2,1);
        next_y = path(2,2);
        if(next_x == dest(1) && next_y == dest(2))
            if ind == 1
                frontier = frontier(2:end,:);
            elseif ind == length(dists)
                frontier = frontier(1:end-1,:);
            else
                frontier = [frontier(1:ind-1,:);frontier(ind+1:end,:)];
            end
        end
    end
end

    