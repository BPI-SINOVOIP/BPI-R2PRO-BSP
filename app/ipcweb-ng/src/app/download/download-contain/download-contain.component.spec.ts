import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DownloadContainComponent } from './download-contain.component';

describe('DownloadContainComponent', () => {
  let component: DownloadContainComponent;
  let fixture: ComponentFixture<DownloadContainComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DownloadContainComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DownloadContainComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
