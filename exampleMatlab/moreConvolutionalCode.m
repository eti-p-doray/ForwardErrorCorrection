function moreConvolutionalCode
%This function shows how to construct a simple convolutional code with more advanced options.
%See 'operation' for encoding and decoding operations

    %We define a trellis structure from a polynomial.
    %   This structure is systematic and recursive.
    trellis = poly2trellis(4, [13, 15], 13)

    %We define a simple code with the trellis.
    %We want the code to use blocs of size 256 bits.
    code = fec.ConvolutionalCode(trellis, 256)

    %We can specify the trellis end type :
    %   ZeroTail | Truncation default = Truncation
    code = fec.ConvolutionalCode(trellis, 256, fec.TrellisEndType.PaddingTail)

    %We can also specify the decoder algorithm (only use for appDecode):
    %   LogMap | MaxLogMap default = MaxLogMap
    code = fec.ConvolutionalCode(trellis, 256, fec.TrellisEndType.PaddingTail, fec.MapType.LogMap)

    %And as all codes, with can change the number of thread used for
    %operations. In this case, we are using 2 threads.
    code = fec.ConvolutionalCode(trellis, 256, fec.TrellisEndType.PaddingTail, fec.MapType.LogMap, 2)

    %Lets now use the code for some operations. Since all code offer the
    %same interface, we call the operation function which will work on any
    %defined code
    operations(code);
end