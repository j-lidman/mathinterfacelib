function [f] = kPPHT2(V, w)
E = nchoose(V);
f = @(A) eval_cut_fn_Hyper(E, w, A);
    


function C = eval_cut_fn_Hyper(E, w, A)
    C = 0;
    for i = 1:size(E, 1)
        found=0;
        for j = 1:size(A,2)
            if((found == 0) && (ismember(A(1,j), E{i,1}) == 1))
               C = C + w(E{i,1});
               found = 1;
            end
        end
    end