#Directory structure
|-data
| |-Annotation **PASCAL annotation file made by make_pascal_dataset_from_masks.py**
| |-PNGImages **images**
| |-Masks **Mask files**
|-checkpoint
| |-...
| |-...
|-out **test output visualized**
| |-...
| |-...
|-train.py
|-inference.py
|-make_pascal_dataset_from_masks.py
|-engine.py
|-utils.py
|-...
|-...

#Data processing
The original GT image files are mask only. Get BB from the mask and generate the
annotation files as per PASCAL format.

**python make_pascal_dataset_from_masks.py**

#Train
**python train**

#Test
Make inference on the dataset. The data are split into training set and testing
set, test on testset and made the prediction result visualized in the filse
unser out directory.
**python inference.py**
