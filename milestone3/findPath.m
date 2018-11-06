function path = findPath(dest,start,walls,path)
    
    wall_bin = de2bi(walls(start(2),start(1)), 4, 'right-msb');
    frontier = [];
    if(wall_bin(1) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = start(1);
        frontier(nr_f+1,2) = start(2) - 1;
    end
    if(wall_bin(2) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = start(1);
        frontier(nr_f+1,2) = start(2)+1;
    end
    if(wall_bin(3) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = start(1)+1;
        frontier(nr_f+1,2) = start(2);
    end
    if(wall_bin(4) == 0)
        [nr_f,~] = size(frontier);
        frontier(nr_f+1,1) = start(1)-1;
        frontier(nr_f+1,2) = start(2);
    end
    
    % Remove already path nodes
    [nr_s,~] = size(path);
    for i = 1:nr_s
        point = path(i,1:2);
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
    
    [nr_f,~] = size(frontier);
    found = 0;
    for c = 1:nr_f
        if isequal(dest,frontier(c,1:2))
            path = [path;dest,1];
            found = 1;
        end
    end
    
    if found == 0
        for c = 1:nr_f
            if c == 1
                new_start = frontier(c,1:2);
                poss = findPath(dest,new_start,walls,path);
                poss_path_best = [new_start, poss(1,3)+1;poss];
            else
                new_start = frontier(c,1:2);
                poss = findPath(dest,new_start,walls,path);
                poss_path = [new_start, poss(1,3)+1;poss];
                if(poss_path_best(1,3) > poss_path(1,3))
                    poss_path_best = poss_path;
                end
            end
        end
        if nr_f == 0
            path = [0,0,100];
        else
            path = poss_path_best;
        end
    end
    
end
