TODO
----

* Write test for massive search documents
* Make association task work for specified doctypes
* Create an AssociationCursor and respective page
* Create a Fragment class and associated page
* JSON versions of appropriate templates
* Implement Document caching
* Refactor paging into class
* Store high frequency matches separately

DONE
----

* ~~Update init.d with correct settings and paths~~
* ~~Make association task multithreaded~~
* ~~Better ordering of search results and fragments within an association~~
* ~~Allow ordering of documents by metadata~~
* ~~Separate queue payload out to speed up queue page and refactor Command to be polymorphic~~
* ~~Implement search page~~
* ~~Implement association task~~
* ~~Create a DocumentSharedPtr and AssociationSharedPtr and pass them around instead~~
* ~~Write tests for each class~~
* ~~Implement gflags parsing~~
* ~~Add separate meta db to speed up document page~~
* ~~Adopt Google C++ Coding standards (eg, getMethods())~~
* ~~Make tests build faster~~


V2
--
* Scatter plots of associations per document (Diagonals equals a cluster)
* Investigate k-means clustering
* Use sparsetable<char[BLOCK]> rather than sparsetable<char*>
* Full curl test of REST API
* Implement unicode using ICU

