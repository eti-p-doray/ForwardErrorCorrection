function LdpcCode
%This function shows how to construct a simple ldpc code.
%See 'operation' for encoding and decoding operations

    %We define low density parity check matrixfrom the dvbs2 standard with a rate
    %1/2
    H = dvbs2ldpc(1/2);
    
    %We define a simple code with the ldpc matrix.
    %The code constructor will check if the matrix is in a triangular shape
    %   (which is the case for dvbs2) and if not, will transforms it to
    %   allow inplace encoding.
    %   The dvbs2 matrices are defined with 64800 column. 
    %   The construction of a code with such a big matrix takes a while
    %   since it needs to ensure the shape allows efficient encoding.
    code = fec.LdpcCode(H)
    
    %Lets now use the code for some operations. Since all code offer the
    %same interface, we call the operation function which will work on any
    %defined code
    operations(code);
end