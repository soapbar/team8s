function segment_out = addSegment(segments,ID,nodeID)

    if ID > nodeID
        nodeB = ID;
        nodeA = nodeID;
    else
        nodeA = ID;
        nodeB = nodeID;
    end

    segmentID= 2*nodeA-sign(nodeB-nodeA-1); %(nodeA < nodeB)
    
    segment_out = [segments;segmentID,nodeA,nodeB];
end



