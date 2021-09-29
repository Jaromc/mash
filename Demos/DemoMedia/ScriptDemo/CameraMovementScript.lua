--Global values maintain their values for the life of the script.

--This value is set when attached to a node.
thisNode = 0

--This value is called and set from the application.
playerControllerId = 1

moveSpeed = 50.0;
rotateSpeed = 1.0;

currentYaw = 0.0
currentPitch = 0.0

--Called when first attached to a node.
function onAttach()
	--Gets the ID of the active node.
	--getNode() will return the scene node that called this function.
	thisNode = scene.getNode();
end

--Called when the scene node is updated.
function onUpdate()
	
	--First updates the nodes movement.
	
	--Gets current controller axis movement.
	x = input.getValue(playerControllerId, key.moveXAxis); 
	y = 0.0; 
	z = input.getValue(playerControllerId, key.moveYAxis);
	
	--Only change the node position if the axis has moved.
	if ((x ~= 0.0) or (z ~= 0.0)) then
		--Calling our custom function.
		x, y, z = user.rotateVectorToNode(thisNode, x, y, z);
		movementVectorLength = math.sqrt((x * x) + (y * y) + (z * z));
		if (movementVectorLength > 1.0) then
			x = x / movementVectorLength;
			y = y / movementVectorLength;
			z = z / movementVectorLength;
		end
		
		--getFixedTime() will scale our movement based on application speed
		moveAmount = moveSpeed * scene.getFixedTime();
		x = x * moveAmount; 
		y = y * moveAmount; 
		z = z * moveAmount;
		scene.addNodePosition(thisNode, x, y, z);
	end
	
	--Update the nodes rotation. Based on axis movement.
	yawMovement = input.getValue(playerControllerId, key.lookXAxis); 
	pitchMovement = -input.getValue(playerControllerId, key.lookYAxis);
	
	if ((yawMovement ~= 0.0) or (pitchMovement ~= 0.0)) then
		rotateAmount = rotateSpeed * scene.getFixedTime()
		currentYaw = currentYaw + (yawMovement * rotateAmount);
		currentPitch = currentPitch + (pitchMovement * rotateAmount);
		
		--Cap the rotation to stop weird things happening. 
		if currentPitch > 1.4 then
			currentPitch = 1.4;
		end
		if currentPitch < -1.4 then
			currentPitch = -1.4;
		end
		
		scene.setNodeRotation(thisNode, currentPitch, currentYaw, 0.0);
	end
end