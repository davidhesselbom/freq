function [data]=matlaboperation_amplify(data)

global state;
if isempty(state) || data.offset==0
    state.counter = 1;
else
    state.counter = state.counter + 1;
end

disp (['amplify #' num2str(state.counter) ' - ' ...
       'data = [' num2str(data.offset/data.samplerate) ', ' num2str((data.offset+numel(data.buffer))/data.samplerate) ') s ' ...
       '[' num2str(data.offset) ', ' num2str(data.offset+numel(data.buffer)) ') ' ...
       'redundancy = ' num2str(data.redundancy) ]);

data.buffer = data.buffer*4;

