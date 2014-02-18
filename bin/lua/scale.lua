
function scale(p, y, r)
	p = p * 0.5
	y = y * 0.5
	r = r * 0.5
	
	y = -y -- Invert this one
	
	if p >= 180 then
		p = p - 360
	elseif p <= -180 then
		p = p + 360
	end
	
	if y >= 180 then
		y = y - 360
	elseif y <= -180 then
		y = y + 360
	end
	
	if r >= 180 then
		r = r - 360
	elseif r <= -180 then
		r = r + 360
	end
	
	return p, y, r
end