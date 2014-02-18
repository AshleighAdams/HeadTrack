local function BSL(x, c)
	return x * (2 ^ c)
end

local function BSR(x, c)
	return x / (2 ^ c)
end

local function log2(x)
	return math.log(x) / math.log(2)
end

_R = _R or debug.getregistry()

_R.EWMA = {}
local meta = {}

function _R.EWMA:Get()
	return BSR(self.internal, self.factor)
end

function _R.EWMA:Set(factor, weight)
	self.factor = log2(factor)
	self.weight = log2(weight)
end

function _R.EWMA:Add(A)
	if self.internal ~= 0 then
		self.internal = BSR(
				BSL(self.internal, self.weight) - self.internal + BSL(A, self.factor)
			, self.weight);
	else
		self.internal = BSL(A, self.factor);
	end
	
	return self
end

function meta:__tostring()
	return "EWMA: " .. tostring(self:Get())
end

function meta:__index(k)
	return _R.EWMA[k]
end



function EWMA(factor, weight)
	local ret = {}
	
	setmetatable(ret, meta)
	
	ret.factor = log2(factor)
	ret.weight = log2(weight)
	ret.internal = 0
	
	return ret
end