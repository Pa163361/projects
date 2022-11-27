#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 */
int column_ind;
bool sortcol(vector<int> &a, vector<int> &b)
{
    if(parsedQuery.sortingStrategy == ASC)
        return a[column_ind] <= b[column_ind];
    return a[column_ind] >= b[column_ind];
}
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    if(((tokenizedQuery.size()!= 8) && (tokenizedQuery.size()!= 10)) || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN"){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    parsedQuery.buffer_size = 10;
    if(tokenizedQuery.size() == 10)
    {
        parsedQuery.buffer_size = stoi(tokenizedQuery[9]);
    }
    string sortingStrategy = tokenizedQuery[7];
    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    // Phase - 1: Sorting
    Table table = *(tableCatalogue.getTable(parsedQuery.sortRelationName));
    column_ind = table.getColumnIndex(parsedQuery.sortColumnName);
    int total_rows = table.rowCount;
    cout<<total_rows<<endl;
    int nB = parsedQuery.buffer_size;
    int B = table.blockCount;
    int times = (int)ceil((float)B/(nB - 1));
    int page_index = 0;
    vector<Page> buffer_blocks(nB-1);

    vector<string> columns;
    for (int columnCounter = 0; columnCounter < table.columnCount; columnCounter++)
    {
        string columnName = table.columns[columnCounter];
        columns.emplace_back(columnName);
    }
    string temp_res_name = "temp_table";
    Table *tempTable = new Table(temp_res_name, columns);

    while(times--)
    {
        int counter = 0;
        // Loading nB-1 blocks into buffer
        while(counter<nB-1 && page_index<B)
        {
            buffer_blocks[counter] = Page(parsedQuery.sortRelationName,page_index);
            counter++;
            page_index++;
        }

        // Do internal sorting
        for(int i=0;i<counter;i++)
        {
            vector<vector<int>> page_data;
            Page cur = buffer_blocks[i];
            for(int rowCounter=0;rowCounter<table.maxRowsPerBlock;rowCounter++)
            {
                vector<int>row = cur.getRow(rowCounter);
                if(row.size()==0) break;
                page_data.push_back(row);
            }
            sort(page_data.begin(),page_data.end(),sortcol);
            for(int rowCounter=0;rowCounter<page_data.size();rowCounter++)
            {
                tempTable->writeRow<int>(page_data[rowCounter]);
            }
        }
    }
    tempTable->blockify();
    tableCatalogue.insertTable(tempTable);

    // Phase - 2: Merging
    Table *resultantTable = new Table(parsedQuery.sortResultRelationName, columns);
    if(parsedQuery.sortingStrategy == ASC)
    {
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> min_heap;
        vector<int> pageRowPointers(tempTable->blockCount,0);
        unordered_map<int,vector<int>> all_rows;

        // Updating required information in min_heap and map
        for(int i=0;i<tempTable->blockCount;i++)
        {
            Page cur = Page(temp_res_name,i);
            vector<int> row = cur.getRow(pageRowPointers[i]);
            all_rows[i] = row;
            min_heap.push({row[column_ind],i});
        }

        while(!min_heap.empty())
        {
            int val, page_no;
            tie(val,page_no) = min_heap.top();
            min_heap.pop();

            resultantTable->writeRow<int>(all_rows[page_no]);
            pageRowPointers[page_no]++;
            if(pageRowPointers[page_no] < tempTable->rowsPerBlockCount[page_no])
            {
                Page cur = Page(temp_res_name,page_no);
                vector<int> row = cur.getRow(pageRowPointers[page_no]);
                all_rows[page_no] = row;
                min_heap.push({row[column_ind],page_no});
            }
        }
    }
    else
    {
        priority_queue<pair<int,int>> max_heap;
        vector<int> pageRowPointers(tempTable->blockCount,0);
        unordered_map<int,vector<int>> all_rows;

        // Updating required information in max_heap and map
        for(int i=0;i<tempTable->blockCount;i++)
        {
            Page cur = Page(temp_res_name,i);
            vector<int> row = cur.getRow(pageRowPointers[i]);
            all_rows[i] = row;
            max_heap.push({row[column_ind],i});
        }

        while(!max_heap.empty())
        {
            int val, page_no;
            tie(val,page_no) = max_heap.top();
            max_heap.pop();

            resultantTable->writeRow<int>(all_rows[page_no]);
            pageRowPointers[page_no]++;
            if(pageRowPointers[page_no] < tempTable->rowsPerBlockCount[page_no])
            {
                Page cur = Page(temp_res_name,page_no);
                vector<int> row = cur.getRow(pageRowPointers[page_no]);
                all_rows[page_no] = row;
                max_heap.push({row[column_ind],page_no});
            }
        }
    }
    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
    tableCatalogue.deleteTable(temp_res_name);
    return;
}