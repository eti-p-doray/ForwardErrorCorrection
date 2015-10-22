%>  This class gathers options affecting puncturing in Turbo Codec.
classdef PunctureOptions < hgsetget
    properties
        %>  Mask for systematic bits and parity bits for each constituent.
        %> This array has k+1 rows or cells with k the number of constituent. The first element is a mask for systematics bits in the form of a column vector. The k next elements are the masks for the output of each k constituents. Every mask is a sequence of 0 and 1, where only ones are kept. The mask sequence is repeated until the end of the parity sequence. Each mask sequence can have a different length by using a cell array instead of a matrix.
        mask = [];
        %>  Mask for systematic tail bits and parity tail bits for each constituent.
        %>  This array has k*2 rows or cells with k the number of constituent. Every (2*i)th element is the mask for systematic bits terminating the ith constituent and every (2*i+1)th element is the mask for parity bits generated by the ith constituent during termination.
        tailMask = [];
        %>  Turbo::BitOreding type that is outputted.
        bitOrdering = uint32(fec.Turbo.BitOrdering.('Alternate'));
    end

    methods
        function self = PunctureOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Turbo.PunctureOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            else
                if (~isempty(varargin))
                    self.set(varargin{:});
                end
            end
        end

        function self = set.bitOrdering(self,val)
            self.bitOrdering = uint32(fec.Turbo.BitOrdering.(val));
        end
    end
end