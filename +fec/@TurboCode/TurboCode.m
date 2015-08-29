classdef TurboCode < fec.Code
    % This class is a specialization of the Code abstract class.
    % It represents a Turbo code.
    % For simplicity, the turbo code parity bits are generated following this structure:
    %   msg | constituent1 | constituent2 | etc.

    properties (Dependent = true)
        iterationCount
    end
    
    methods (Static)
        function b = loadobj(a)
        % Overload of the loadobj
        %   Allocate underlying ressources from saved configuration.
        %
            b = fec.TurboCode();
            b.load(a);
        end
    end

    methods
        function count = get.iterationCount(this)
            count = fec.bin.TurboCode_get_iterationCount(this);
        end
        function set.iterationCount(this, count)
            fec.bin.TurboCode_set_iterationCount(this, count);
        end
        
        function this = TurboCode(trellis, interleaver, terminationType, iterationCount, schedulingType, mapDecoderType, workGroupSize)
        % TurboCode constructor
        %   Configures the object internally and allocate cpp ressources
        %
        % Inputs
        %   trellis - Trellis structure used by constituent codes in the matlab communication system toolox form
        %   interleaver - Interleaver used by the second code.
        %   [iterationCount] - Number of iterations. default = 5
        %   [endType] - Trellis end type for every constituent codes. PaddingTail | Truncation default = PaddingTail
        %   [structureType] - Decoder algorithm type. This defines the scheduling of extrinsic communication between code
        %       constituents. Serial | Parallel default = Serial
        %   [mapDecoderType] - decoder algorithm type. LogMap | MaxLogMap default = MaxLogMap
        %   [workGroupSize] - Number of thread used in decodage. default = 4
        %
          if (nargin == 0)
              return;
          end
          if (nargin < 3)
              endType = fec.TerminationType.Tail;
          end
          if (nargin < 4)
              iterationCount = 5;
          end
          if (nargin < 5)
            schedulingType = fec.SchedulingType.Serial;
          end
          if (nargin < 6)
            mapDecoderType = fec.MapType.MaxLogMap;
          end
          if (nargin < 7)
            workGroupSize = 8;
          end
          if (length(trellis) == 1)
            trellis = repmat({trellis}, size(interleaver));
          end
          if (length(terminationType) == 1)
            terminationType = repmat({terminationType}, size(interleaver));
          end
          terminationTypeChar = cell(size(terminationType));
          for i = 1:length(terminationType)
              terminationTypeChar{i} = terminationType{i}.char;
          end
          this.mexHandle_ = fec.bin.TurboCode_constructor(trellis, interleaver, terminationTypeChar, iterationCount, schedulingType.char, mapDecoderType.char, workGroupSize);
        end
    end
end